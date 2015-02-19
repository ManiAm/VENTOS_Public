
package c3po.ned;

simple SumoBinary
{
    parameters:
        @class(VENTOS::SumoBinary);
        @display("i=block/network2");
        
        bool update;
        
        string SUMO_GUI_FileName;
        string SUMO_CMD_FileName;
        string SUMO_GUI_URL;
        string SUMO_CMD_URL;
        string SUMO_Version_URL;
}

