#include "EthernetApplication.h"
#include "EthernetDataFrames_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/common/PcpTag_m.h"
#include "inet/common/ProtocolUtils.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "TSNSocket.h"

Define_Module(EthernetApplication);

void EthernetApplication::initialize(int stage) {
    ApplicationBase::initialize(stage);

    if(stage == INITSTAGE_LOCAL) {
        socket.setCallback(this); //impostiamo come callback l'oggetto me stesso, perchè a me stesso gli ho fatto implementare l'interfaccia TSNSocket::ICallBack, interfaccia con 3 metodi
        socket.setOutputGate(gate("socketOut")); //come outputGate indichiamo qual è il gate che fa l'output, per inviare messaggi

    } else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION+1) { //prendiamo l'interface_configuration+1 perchè così attendiamo che tutte le interfacce di rete siano configurate
        const char *localAddressString = par("localAddress");
        if (*localAddressString != '\0') {
            L3Address l3Address;
            L3AddressResolver addressResolver; //L3AddressResolver è un modulo di INET che converte il nome di un nodo nell'indirizzo MAC di quel nodo specifico
            addressResolver.tryResolve(localAddressString, l3Address, L3AddressResolver::ADDR_MAC);
            if (l3Address.getType() == L3Address::MAC)
                localAddress = l3Address.toMac();
            else
                localAddress = MacAddress(localAddressString);
        }
        const char *remoteAddressString = par("remoteAddress");
        if (*remoteAddressString != '\0') {
            L3Address l3Address;
            L3AddressResolver addressResolver;
            addressResolver.tryResolve(remoteAddressString, l3Address, L3AddressResolver::ADDR_MAC);
            if (l3Address.getType() == L3Address::MAC)
                remoteAddress = l3Address.toMac();
            else
                remoteAddress = MacAddress(remoteAddressString);
        }

        auto interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        networkInterface = interfaceTable->findInterfaceByName(par("netInterface"));
        if (networkInterface == nullptr)
            throw cRuntimeError("Cannot find network interface");
        if (!localAddress.isUnspecified())
            networkInterface->addMulticastMacAddress(localAddress);
    }
}

void EthernetApplication::handleMessageWhenUp(cMessage *msg) {
    if(msg->isSelfMessage()) {
        if(strcmp(msg->getName(), "StartTimer") ==  0) {
            generateFrames();
            delete msg;
            cMessage *tim = new cMessage("GenTimer");
            scheduleAt(par("period")+simTime(), tim);
            return;
        } else if(strcmp(msg->getName(), "GenTimer") ==  0) {
            generateFrames();
            scheduleAt(par("period")+simTime(), msg);
            return;
        } else if(strcmp(msg->getName(), "InterTimer") ==  0) {
            delete msg;
            transmitFrames();
            return;
        }
    }

    //se non è un Self Message controlliamo che il messaggio appartenga ad un Socket
    if (socket.belongsToSocket(msg)) {
        //se il messaggio appertiene ad un Socket chiamiamo il metodo processMessage, che chiamerà la callback socketDataArrived
        socket.processMessage(msg);
        return;
    }



    delete msg;
}

void EthernetApplication::generateFrames() {
    int remLen = par("payload");
    int mtu = par("mtu");
    Packet *pkt = nullptr;
    int num = 0;
    int len;
    int pcp = par("priority");
    int vlanid = par("vlanid");

    int burst = ceil((double)remLen/(double)mtu);

    while(remLen > 0) {
        pkt = new Packet(par("name").stringValue());

        //quando inviamo un messaggio, quando lo generiamo, leggiamo prima il VlanId, il Pcp, la destinazione e l'interfaccia su cui devo trasmettere
        //vengono inseriti dei tag per ognuno di questi parametri
        pkt->addTag<VlanReq>()->setVlanId(vlanid);
        pkt->addTag<PcpReq>()->setPcp(pcp);
        pkt->addTag<MacAddressReq>()->setDestAddress(remoteAddress);
        pkt->addTag<InterfaceReq>()->setInterfaceId(networkInterface->getInterfaceId());

        auto data = makeShared<EthernetDataPayload>();
        data->setFrame_num(num);
        data->setBurst_size(burst);
        data->setGenTime(simTime());
        num++;
        len = (remLen < mtu) ? remLen : mtu;
        data->setChunkLength(B(len));
        pkt->insertAtBack(data);


        queue.insert(pkt);
        remLen -= len;
    }

    transmitFrames(); //FIXME Si assume che la trasmissione del burst termini prima del periodo
}

void EthernetApplication::socketDataArrived(TSNSocket *socket, Packet *pkt) {
    /* Messaggio dalla rete */
    EV << "Arrivato pacchetto: " << pkt->getName() << endl;
    auto pay = pkt->peekData<EthernetDataPayload>();
    int bs = pay->getBurst_size();
    int num = pay->getFrame_num();
    simtime_t e2ed;
    simsignal_t sig;
    if((bs-1) == num) {
        e2ed = simTime()-pay->getGenTime();
        sig = registerSignal("BurstDelay");
        emit(sig, e2ed);
    }

    e2ed = simTime()-pay->getGenTime();
    sig = registerSignal("FrameDelay");
    emit(sig, e2ed);
    delete pkt;
}

void EthernetApplication::transmitFrames() {
    simtime_t it = par("interarrivalTime");
    if(!queue.isEmpty()) {
        auto pkt = queue.pop();
        socket.send(check_and_cast<Packet *>(pkt));

        if(!queue.isEmpty()) {
            if(it == 0) {
                transmitFrames();
            } else {
                cMessage *tim = new cMessage("InterTimer");
                scheduleAt(simTime()+it, tim);
            }
        }
    }
}

void EthernetApplication::handleStartOperation(LifecycleOperation *operation) {
    simtime_t stime = par("startTime");

    socket.bind(par("name").stringValue());

    if(stime >= 0) {
        cMessage *tim = new cMessage("StartTimer");
        scheduleAt(simTime()+stime, tim);
    }
}

void EthernetApplication::handleStopOperation(LifecycleOperation *operation) {

}

void EthernetApplication::handleCrashOperation(LifecycleOperation *operation) {

}

