package c3po.ned;


simple ApplVPlatoonMg extends ApplVPlatoonFormed
{
    parameters:
        @class(ApplVPlatoonMg); 
        
        int maxPlatoonSize;
        int optPlatoonSize;
        
        bool mergeEnabled;
        bool splitEnabled;        
}
