/****************************************************************************/
/// @file    test_init.c
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author
/// @date
///
/****************************************************************************/
// @section LICENSE
//
// This software embodies materials and concepts that are confidential to Redpine
// Signals and is made available solely pursuant to the terms of a written license
// agreement with Redpine Signals
//


int main(void)
{
    signal(SIGINT, sigint);

    return 0;
}


void sigint(int signum)
{

    exit(0);
}
