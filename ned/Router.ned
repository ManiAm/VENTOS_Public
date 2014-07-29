
package c3po.ned;

simple Router
{
    parameters:
        @class(VENTOS::Router);
        @display("i=block/network2");        
        
        bool enableRouting = default(false); // build router-side networks 
}

