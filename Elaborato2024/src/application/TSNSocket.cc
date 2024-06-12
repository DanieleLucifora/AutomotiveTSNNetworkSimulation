#include "TSNSocket.h"
#include "EthernetDataFrames_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/socket/SocketTag_m.h"

void TSNSocket::sendOut(cMessage *msg) { //richiama la sendOut che aveva fatto la classe ereditata
    SocketBase::sendOut(msg);
}

void TSNSocket::sendOut(Request *request) { //aggiunge nel dispatch protocol il protocollo TSN, serve a far si che la bind vada a finire nel modulo TSNNetLayer
    request->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::tsn);
    SocketBase::sendOut(request);
}

void TSNSocket::sendOut(Packet *packet) { //un pacchetto trasmesso attraverso questo socket deve avere il protocol dispatchProtocol request : tsn
    packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::tsn);
    SocketBase::sendOut(packet);
}

void TSNSocket::bind(const char *flowName) { //quando l'applicazione chiamerà la funzione bind passerà il nome del flusso
    auto request = new Request("BIND", SOCKET_C_BIND); //viene creata una nuova richiesta
    TSNBindCommand *ctrl = new TSNBindCommand();
    ctrl->setFlowName(flowName);
    request->setControlInfo(ctrl); //nelle controlInfo andiamo ad agganciare la richiesta ctrl. Il ControlInfo è puntatore di tipo C object che è inserito come attributo all'interno di un C message. Serve per agganciare informazioni aggiuntive al messaggio utilizzando un puntatore libero.
    isOpen_ = true;
    sendOut(request); //lo invia giù al livello sottostante
}

void TSNSocket::processMessage(cMessage *msg) { //controlla se il socket ha I_DATA, cioè se è un indication, cioè un messaggio che arriva dal network layer
    ASSERT(belongsToSocket(msg));
    switch (msg->getKind()) {
        case SOCKET_I_DATA: //se ha I_DATA chiama la callback socketDataArrived
            if (callback)
                callback->socketDataArrived(this, check_and_cast<Packet *>(msg));
            else
                delete msg;
            break;
        case SOCKET_I_CLOSED: //se è un messaggio indication closed, chiama la callback socketClosed
            isOpen_ = false;
            if (callback)
                callback->socketClosed(this);
            delete msg;
            break;
        default:
            throw cRuntimeError("EthernetSocket: invalid msg kind %d, one of the ETHERNNET_I_xxx constants expected", msg->getKind());
            break;
    }
}
