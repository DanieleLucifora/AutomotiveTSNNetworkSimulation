#include "TSNNetLayer.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/common/ProtocolUtils.h"
#include "EthernetDataFrames_m.h"
#include "inet/common/socket/SocketBase.h"

Define_Module(TSNNetLayer);

void TSNNetLayer::initialize(int stage) {
    if(stage == INITSTAGE_NETWORK_LAYER) {
        //il metodo registerProtocol indica ai messageDispatcher che se c'è un protocollo chiamato tsn lo devi inviare, in uscita al gate ifOut
        //in ingresso al gate ifIn
        //invece i servizi sono come i protocolli ma sono per i dati che provengono dalle applicazioni
        registerProtocol(Protocol::tsn, gate("ifOut"), gate("ifIn"));
        registerService(Protocol::tsn, gate("transportOut"), gate("transportIn"));
    }
}

void TSNNetLayer::handleMessage(cMessage *msg) {
    //se arriva un pacchetto da livello trasporto, quindi dal livello superiore, ed è un pacchetto dati
    if(msg->arrivedOn("transportIn")) {
        if(msg->isPacket()) {
            Packet *pkt = check_and_cast<Packet *>(msg);
            pkt->addTag<PacketProtocolTag>()->setProtocol(&Protocol::tsn); //aggiungiamo un protocolTag -> tsn
            ensureEncapsulationProtocolReq(pkt, &Protocol::ieee8021qCTag); //incapsuliamo un protocolTag ieee8021qCTag, per far si che il pacchetto vada a finire nel livello MAC
            setDispatchProtocol(pkt);
            send(pkt, "ifOut"); //invia il pacchetto al livello sottostante
            return;
        } else { //se il messaggio arrivato non è un pacchetto può essere un comando
            Request *req = check_and_cast<Request *>(msg);
            //se il messaggio è un comando, cioè l'applicazione che fa la bind sul socket
            //il comando bind è stato creato nel EthernetDataFrames.msg
            if (auto bindCommand = dynamic_cast<TSNBindCommand *>(req->getControlInfo())) {
                int socketId = check_and_cast<Request *>(msg)->getTag<SocketReq>()->getSocketId();
                //creaiamo una struct di tipo Socket
                Socket *socket = new Socket(socketId);
                //settiamo il campo flowName
                socket->flowName = bindCommand->getFlowName();
                //inseriamo nella mappa il socket con il socketId
                socketIdToSocketMap[socketId] = socket;
            }
            delete msg;
            return;
        }
    } else { //se arriva un pacchetto da sotto
        //facciamo il check_and_cast al Packet
        Packet *pkt = check_and_cast<Packet *>(msg);
        for (auto it : socketIdToSocketMap) { //scorriamo la mappa dei Socket
            auto socket = it.second;
            if (socket->matches(pkt)) { //se c'è un socket che fa match, cioè che il nome del pacchetto è uguale al flowname del socket
                auto packetCopy = pkt->dup();
                packetCopy->setKind(SOCKET_I_DATA);
                packetCopy->addTagIfAbsent<SocketInd>()->setSocketId(it.first); //aggiungiamo un tag di tipo socketIndication
                EV_INFO << "Sending " << packetCopy << " to socket " << it.first << ".\n"; //inviamo il pacchetto ai livelli superiori
                send(packetCopy, "transportOut");
            }
        }
        delete msg;
        return;
    }
}

bool TSNNetLayer::Socket::matches(Packet *packet) {
    if (strcmp(packet->getName(), flowName.c_str()) == 0) {
        return true;
    }
    return false;
}
