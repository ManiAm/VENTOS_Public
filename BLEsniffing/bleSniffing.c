#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#define DEFAULT_COMPID	65535

struct frame {
	void		*data;
	uint32_t	data_len;
	void		*ptr;
	uint32_t	len;
	uint16_t	dev_id;
	uint8_t		in;
	uint8_t		master;
	uint16_t	handle;
	uint16_t	cid;
	uint16_t	num;
	uint8_t		dlci;
	uint8_t		channel;
	unsigned long	flags;
	struct timeval	ts;
	int		pppdump_fd;
	int		audio_fd;
};

/* Parser flags */
#define DUMP_WIDTH	20

#define DUMP_ASCII	0x0001
#define DUMP_HEX	0x0002
#define DUMP_EXT	0x0004
#define DUMP_RAW	0x0008
#define DUMP_BPA	0x0010
#define DUMP_TSTAMP	0x0100
#define DUMP_VERBOSE	0x0200
#define DUMP_BTSNOOP	0x1000
#define DUMP_PKTLOG	0x2000
#define DUMP_NOVENDOR	0x4000
#define DUMP_TYPE_MASK	(DUMP_ASCII | DUMP_HEX | DUMP_EXT)

#define SNAP_LEN	HCI_MAX_FRAME_SIZE

#define LE_LINK		0x03

#define FLAGS_AD_TYPE 0x01
#define FLAGS_LIMITED_MODE_BIT 0x01
#define FLAGS_GENERAL_MODE_BIT 0x02

#define EIR_FLAGS                   0x01  /* flags */
#define EIR_UUID16_SOME             0x02  /* 16-bit UUID, more available */
#define EIR_UUID16_ALL              0x03  /* 16-bit UUID, all listed */
#define EIR_UUID32_SOME             0x04  /* 32-bit UUID, more available */
#define EIR_UUID32_ALL              0x05  /* 32-bit UUID, all listed */
#define EIR_UUID128_SOME            0x06  /* 128-bit UUID, more available */
#define EIR_UUID128_ALL             0x07  /* 128-bit UUID, all listed */
#define EIR_NAME_SHORT              0x08  /* shortened local name */
#define EIR_NAME_COMPLETE           0x09  /* complete local name */
#define EIR_TX_POWER                0x0A  /* transmit power level */
#define EIR_DEVICE_ID               0x10  /* device ID */


/* /////////////////////////////LE SCAN////////////////////////////////////// */
static volatile int signal_received = 0;

static void sigint_handler(int sig)
{
	signal_received = sig;
}

static int read_flags(uint8_t *flags, const uint8_t *data, size_t size)
{
	size_t offset;

	if (!flags || !data)
		return -EINVAL;

	offset = 0;
	while (offset < size) {
		uint8_t len = data[offset];
		uint8_t type;

		/* Check if it is the end of the significant part */
		if (len == 0)
			break;

		if (len + offset > size)
			break;

		type = data[offset + 1];

		if (type == FLAGS_AD_TYPE) {
			*flags = data[offset + 2];
			return 0;
		}

		offset += 1 + len;
	}

	return -ENOENT;
}

static void eir_parse_name(uint8_t *eir, size_t eir_len,
						char *buf, size_t buf_len)
{
	size_t offset;

	offset = 0;
	while (offset < eir_len) {
		uint8_t field_len = eir[0];
		size_t name_len;

		/* Check for the end of EIR */
		if (field_len == 0)
			break;

		if (offset + field_len > eir_len)
			goto failed;

		switch (eir[1]) {
		case EIR_NAME_SHORT:
		case EIR_NAME_COMPLETE:
			name_len = field_len - 1;
			if (name_len > buf_len)
				goto failed;

			memcpy(buf, &eir[2], name_len);
			return;
		}

		offset += field_len + 1;
		eir += field_len + 1;
	}

failed:
	snprintf(buf, buf_len, "(unknown)");
}

static int check_report_filter(uint8_t procedure, le_advertising_info *info)
{
	uint8_t flags;

	/* If no discovery procedure is set, all reports are treat as valid */
	if (procedure == 0)
		return 1;

	/* Read flags AD type value from the advertising report if it exists */
	if (read_flags(&flags, info->data, info->length))
		return 0;

	switch (procedure) {
	case 'l': /* Limited Discovery Procedure */
		if (flags & FLAGS_LIMITED_MODE_BIT)
			return 1;
		break;
	case 'g': /* General Discovery Procedure */
		if (flags & (FLAGS_LIMITED_MODE_BIT | FLAGS_GENERAL_MODE_BIT))
			return 1;
		break;
	default:
		fprintf(stderr, "Unknown discovery procedure\n");
	}

	return 0;
}

static int print_advertising_devices(int dd, uint8_t filter_type)
{
	unsigned char buf[HCI_MAX_EVENT_SIZE], *ptr;
	struct hci_filter nf, of;
	struct sigaction sa;
	socklen_t olen;
	int len;

	olen = sizeof(of);
	if (getsockopt(dd, SOL_HCI, HCI_FILTER, &of, &olen) < 0) {
		printf("Could not get socket options\n");
		return -1;
	}

	hci_filter_clear(&nf);
	hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
	hci_filter_set_event(EVT_LE_META_EVENT, &nf);

	if (setsockopt(dd, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0) {
		printf("Could not set socket options\n");
		return -1;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_flags = SA_NOCLDSTOP;
	sa.sa_handler = sigint_handler;
	sigaction(SIGINT, &sa, NULL);

	while (1) {
		evt_le_meta_event *meta;
		le_advertising_info *info;
		char addr[18];

		while ((len = read(dd, buf, sizeof(buf))) < 0) {
			if (errno == EINTR && signal_received == SIGINT) {
				len = 0;
				goto done;
			}

			if (errno == EAGAIN || errno == EINTR)
				continue;
			goto done;
		}

		ptr = buf + (1 + HCI_EVENT_HDR_SIZE);
		len -= (1 + HCI_EVENT_HDR_SIZE);

		meta = (void *) ptr;

		if (meta->subevent != 0x02)
			goto done;

		/* Ignoring multiple reports */
		info = (le_advertising_info *) (meta->data + 1);
		if (check_report_filter(filter_type, info)) {
			char name[30];

			memset(name, 0, sizeof(name));

			ba2str(&info->bdaddr, addr);
			eir_parse_name(info->data, info->length,
							name, sizeof(name) - 1);

			printf("%s %s\n", addr, name);
		}
	}

done:
	setsockopt(dd, SOL_HCI, HCI_FILTER, &of, sizeof(of));

	if (len < 0)
		return -1;

	return 0;
}

static void lescan_dup(int dev_id)
{
	int err, opt, dd;
	uint8_t own_type = 0x00;
	uint8_t scan_type = 0x01;
	uint8_t filter_type = 0;
	uint8_t filter_policy = 0x00;
	uint16_t interval = htobs(0x0010);
	uint16_t window = htobs(0x0010);
	uint8_t filter_dup = 0x00;

	if (dev_id < 0)
		dev_id = hci_get_route(NULL);

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("Could not open device");
		exit(1);
	}

	err = hci_le_set_scan_parameters(dd, scan_type, interval, window,
						own_type, filter_policy, 1000);
	if (err < 0) {
		perror("Set scan parameters failed");
		exit(1);
	}

	err = hci_le_set_scan_enable(dd, 0x01, filter_dup, 1000);
	if (err < 0) {
		perror("Enable scan failed");
		exit(1);
	}

	printf("LE Scan ...\n");

	// err = print_advertising_devices(dd, filter_type);
	// if (err < 0) {
	// 	perror("Could not receive advertising events");
	// 	exit(1);
	// }

	// err = hci_le_set_scan_enable(dd, 0x00, filter_dup, 1000);
	// if (err < 0) {
	// 	perror("Disable scan failed");
	// 	exit(1);
	// }

	// hci_close_dev(dd);
}

/* /////////////////////////////////DUMP RAW///////////////////////////////// */

/* Modes */
enum {
	PARSE,
	READ,
	WRITE,
	PPPDUMP,
	AUDIO
};

/* Default options */
static int  snap_len = SNAP_LEN;
static int  mode = PARSE;
static int  permcheck = 1;
static char *dump_file = NULL;
static char *pppdump_file = NULL;
static char *audio_file = NULL;

struct hcidump_hdr {
	uint16_t	len;
	uint8_t		in;
	uint8_t		pad;
	uint32_t	ts_sec;
	uint32_t	ts_usec;
} __attribute__ ((packed));
#define HCIDUMP_HDR_SIZE (sizeof(struct hcidump_hdr)) 

struct btsnoop_hdr {
	uint8_t		id[8];		/* Identification Pattern */
	uint32_t	version;	/* Version Number = 1 */
	uint32_t	type;		/* Datalink Type */
} __attribute__ ((packed));
#define BTSNOOP_HDR_SIZE (sizeof(struct btsnoop_hdr))

struct btsnoop_pkt {
	uint32_t	size;		/* Original Length */
	uint32_t	len;		/* Included Length */
	uint32_t	flags;		/* Packet Flags */
	uint32_t	drops;		/* Cumulative Drops */
	uint64_t	ts;		/* Timestamp microseconds */
	uint8_t		data[0];	/* Packet Data */
} __attribute__ ((packed));
#define BTSNOOP_PKT_SIZE (sizeof(struct btsnoop_pkt))

static uint8_t btsnoop_id[] = { 0x62, 0x74, 0x73, 0x6e, 0x6f, 0x6f, 0x70, 0x00 };

static uint32_t btsnoop_version = 0;
static uint32_t btsnoop_type = 0;

struct pktlog_hdr {
	uint32_t	len;
	uint64_t	ts;
	uint8_t		type;
} __attribute__ ((packed));
#define PKTLOG_HDR_SIZE (sizeof(struct pktlog_hdr))

struct parser_t {
	unsigned long flags;
	unsigned long filter;
	unsigned short defpsm;
	unsigned short defcompid;
	int state;
	int pppdump_fd;
	int audio_fd;
};

struct parser_t parser;

void init_parser(unsigned long flags, unsigned long filter,
		unsigned short defpsm, unsigned short defcompid,
		int pppdump_fd, int audio_fd)
{
	if ((flags & DUMP_RAW) && !(flags & DUMP_TYPE_MASK))
		flags |= DUMP_HEX;

	parser.flags      = flags;
	parser.filter     = filter;
	parser.defpsm     = defpsm;
	parser.defcompid  = defcompid;
	parser.state      = 0;
	parser.pppdump_fd = pppdump_fd;
	parser.audio_fd   = audio_fd;
}

static int open_socket(int dev, unsigned long flags)
{
	struct sockaddr_hci addr;
	struct hci_filter flt;
	struct hci_dev_info di;
	int sk, dd, opt;

	if (permcheck && dev != HCI_DEV_NONE) {
		dd = hci_open_dev(dev);
		if (dd < 0) {
			perror("Can't open device");
			return -1;
		}

		if (hci_devinfo(dev, &di) < 0) {
			perror("Can't get device info");
			return -1;
		}

		opt = hci_test_bit(HCI_RAW, &di.flags);
		if (ioctl(dd, HCISETRAW, opt) < 0) {
			if (errno == EACCES) {
				perror("Can't access device");
				return -1;
			}
		}

		hci_close_dev(dd);
	}

	/* Create HCI socket */
	sk = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	if (sk < 0) {
		perror("Can't create raw socket");
		return -1;
	}

	opt = 1;
	if (setsockopt(sk, SOL_HCI, HCI_DATA_DIR, &opt, sizeof(opt)) < 0) {
		perror("Can't enable data direction info");
		return -1;
	}

	opt = 1;
	if (setsockopt(sk, SOL_HCI, HCI_TIME_STAMP, &opt, sizeof(opt)) < 0) {
		perror("Can't enable time stamp");
		return -1;
	}

	/* Setup filter */
	hci_filter_clear(&flt);
	hci_filter_all_ptypes(&flt);
	hci_filter_all_events(&flt);
	if (setsockopt(sk, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0) {
		perror("Can't set filter");
		return -1;
	}

	/* Bind socket to the HCI device */
	memset(&addr, 0, sizeof(addr));
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = dev;
	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		printf("Can't attach to device hci%d. %s(%d)\n",
					dev, strerror(errno), errno);
		return -1;
	}

	return sk;
}

static int process_frames(int dev, int sock, int fd, unsigned long flags)
{
	struct cmsghdr *cmsg;
	struct msghdr msg;
	struct iovec  iv;
	struct hcidump_hdr *dh;
	struct btsnoop_pkt *dp;
	struct frame frm;
	struct pollfd fds[2];
	int nfds = 0;
	char *buf, *ctrl;
	int len, hdr_size = HCIDUMP_HDR_SIZE;

	if (sock < 0)
		return -1;

	if (snap_len < SNAP_LEN)
		snap_len = SNAP_LEN;

	if (flags & DUMP_BTSNOOP)
		hdr_size = BTSNOOP_PKT_SIZE;

	buf = malloc(snap_len + hdr_size);
	if (!buf) {
		perror("Can't allocate data buffer");
		return -1;
	}

	dh = (void *) buf;
	dp = (void *) buf;
	frm.data = buf + hdr_size;

	ctrl = malloc(100);
	if (!ctrl) {
		free(buf);
		perror("Can't allocate control buffer");
		return -1;
	}

	if (dev == HCI_DEV_NONE)
		printf("system: ");
	else
		printf("device: hci%d ", dev);

	printf("snap_len: %d filter: 0x%lx\n", snap_len, parser.filter);

	memset(&msg, 0, sizeof(msg));

	fds[nfds].fd = sock;
	fds[nfds].events = POLLIN;
	fds[nfds].revents = 0;
	nfds++;

	while (1) {
		int i, n = poll(fds, nfds, -1);
		if (n <= 0)
			continue;

		for (i = 0; i < nfds; i++) {
			if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
				if (fds[i].fd == sock)
					printf("device: disconnected\n");
				else
					printf("client: disconnect\n");
				return 0;
			}
		}

		iv.iov_base = frm.data;
		iv.iov_len  = snap_len;

		msg.msg_iov = &iv;
		msg.msg_iovlen = 1;
		msg.msg_control = ctrl;
		msg.msg_controllen = 100;

		len = recvmsg(sock, &msg, MSG_DONTWAIT);
		if (len < 0) {
			if (errno == EAGAIN || errno == EINTR)
				continue;
			perror("Receive failed");
			return -1;
		}

		/* Process control message */
		frm.data_len = len;
		frm.dev_id = dev;
		frm.in = 0;
		frm.pppdump_fd = parser.pppdump_fd;
		frm.audio_fd   = parser.audio_fd;

		cmsg = CMSG_FIRSTHDR(&msg);
		while (cmsg) {
			int dir;
			switch (cmsg->cmsg_type) {
			case HCI_CMSG_DIR:
				memcpy(&dir, CMSG_DATA(cmsg), sizeof(int));
				frm.in = (uint8_t) dir;
				break;
			case HCI_CMSG_TSTAMP:
				memcpy(&frm.ts, CMSG_DATA(cmsg),
						sizeof(struct timeval));
				break;
			}
			cmsg = CMSG_NXTHDR(&msg, cmsg);
		}

		frm.ptr = frm.data;
		frm.len = frm.data_len;

		unsigned char *buf = frm.ptr;
                int num = -1;
		register int j, m;

		if ((num < 0) || (num > (int) frm.len))
		num = frm.len;

		for (j = 0, m = 1; j < num; j++, m++) {
			printf("%2.2X ", buf[j]);
			if (m == DUMP_WIDTH) {
				printf("\n");
				m = 0;
			}
		}
		if (j && m != 1)
			printf("\n");

	}
	return 0;
}


void main(){
	unsigned long flags = 0;
	flags |= DUMP_RAW;
	unsigned long filter = 0;
	int device = hci_get_route(NULL);
	int defpsm = 0;
	int defcompid = DEFAULT_COMPID;
	int opt, pppdump_fd = -1, audio_fd = -1;
	uint16_t obex_port;

	if (!filter)
		filter = ~0L;

	flags |= DUMP_VERBOSE;
	init_parser(flags, filter, defpsm, defcompid, pppdump_fd, audio_fd);
	lescan_dup(device);
	process_frames(device, open_socket(device, flags), -1, flags);
}
