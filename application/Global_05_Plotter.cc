
#include <Global_05_Plotter.h>

Define_Module(Plotter);

// Note that gnuplot binary must be on the path
// and on Windows we need to use the piped version of gnuplot
#ifdef WIN32
    #define GNUPLOT_NAME "pgnuplot -persist"
#else
    #define GNUPLOT_NAME "gnuplot"
#endif

Plotter::Plotter()
{

}


void Plotter::initialize(int stage)
{
    if(stage == 0)
	{
        on = par("on").boolValue();

        if(!on)
            return;

        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);

        #ifdef WIN32
            pipe = _popen(GNUPLOT_NAME, "w");
        #else
            pipe = popen(GNUPLOT_NAME, "w");
        #endif

        if(pipe == NULL)
            error("Could not open pipe!");



        // set the terminal
        // persist: keep the windows open even on simulation termination
        // noraise: updating is done in the background
        fprintf(pipe, "set term wx 0 noraise\n");

        fprintf(pipe, "set title 'minimum'\n");
        fprintf(pipe, "set xlabel 'Sim Time'\n");
        fprintf(pipe, "set ylabel 'Vehicle Speed'\n");

        // plot type
        fprintf(pipe, "plot '-' with lines\n");

        // loop over the data [0,...,9]. data terminated with \n
        for(int i = 0; i < 10; i++)
            fprintf(pipe, "%d\n", i);

        // termination character
        fprintf(pipe, "%s\n", "e");

        // flush the pipe
        fflush(pipe);

        fprintf(pipe, "set multiplot\n");

        fprintf(pipe, "plot 2*x\n");

        fflush(pipe);


    }
}


void Plotter::handleMessage(cMessage *msg)
{

}


void Plotter::speedProfile()
{
    if(!on)
        return;


}


void Plotter::finish()
{
    if(!on)
        return;

    #ifdef WIN32
        _pclose(pipe);
    #else
        pclose(pipe);
    #endif
}


Plotter::~Plotter()
{

}
