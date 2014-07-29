
package c3po.ned;


simple RSUAdd
{
    parameters:
        @class(VENTOS::RSUAdd);
        @display("i=block/network2");
        
        bool on = default(true);
        int mode;  
        string RSUfile = default("");
}

