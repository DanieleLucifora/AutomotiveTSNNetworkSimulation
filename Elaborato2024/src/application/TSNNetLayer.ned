package elaborato2024.application;

import inet.networklayer.contract.INetworkLayer;

//TSN Network Layer implementa l'interfaccia INetwork Layer, cioè ha i gates transportIn e transportOut che sono i gates verso su 
//mentre ifIn(interface IN) e ifPut(interface OUT) sono i gates verso giù
simple TSNNetLayer like INetworkLayer {
    gates:
        input transportIn @labels(ITransportPacket/down);
        output transportOut @labels(ITransportPacket/up);
        input ifIn @labels(INetworkHeader/up);
        output ifOut @labels(INetworkHeader/down);
}
