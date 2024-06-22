package elaborato2024.simulations;

import inet.networks.base.WiredNetworkBase;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.Eth1G;
import inet.node.tsn.TsnDevice;
import inet.node.inet.StandardHost;
import inet.node.tsn.TsnSwitch;


network TestNet extends WiredNetworkBase
{
    types:
        channel C extends ned.DatarateChannel
        {
            datarate = 100Mbps;
            ber = 1e-8;
        }

    submodules:
        LD2: TsnDevice {
            @display("p=246.64876,99.11");
        }
        US2: TsnDevice {
            @display("p=246.64876,189.21");
        }
        CM1: TsnDevice {
            @display("p=247.126,279.18");
        }
        LD1: TsnDevice {
            @display("p=247.126,369.138");
        }
        US1: TsnDevice {
            @display("p=247.126,446.68802");
        }
        CU: TsnDevice {
            @display("p=495,200");
        }
        S2: TsnDevice {
            @display("p=410.49802,141.658");
        }
        S1: TsnDevice {
            @display("p=410.49802,399.12402");
        }
        HU: TsnDevice {
            @display("p=495.286,315.37");
        }
        S4: TsnDevice {
            @display("p=690.71204,141.658");
        }
        Switch1: TsnSwitch {
            @display("p=410.49802,278.146");
        }
        Switch2: TsnSwitch {
            @display("p=690.71204,278.146");
        }
        S3: TsnDevice {
            @display("p=690.71204,399.12402");
        }
        RC: TsnDevice {
            @display("p=852.016,272.976");
        }
        TLM: TsnDevice {
            @display("p=852.016,362.93402");
        }
        US4: TsnDevice {
            @display("p=852.016,440.484");
        }
        US3: TsnDevice {
            @display("p=852.016,99.264");
        }
        ME: TsnDevice {
            @display("p=852.016,189.222");
        }
        RS2: TsnDevice {
            @display("p=587.312,342.254");
        }
        RS1: TsnDevice {
            @display("p=587.312,199.56201");
        }
    connections:

        Switch1.ethg++ <--> C <--> CU.ethg++;
        Switch1.ethg++ <--> C <--> S2.ethg++;
        Switch1.ethg++ <--> C <--> S1.ethg++;
        Switch1.ethg++ <--> C <--> HU.ethg++;
        Switch1.ethg++ <--> C <--> US1.ethg++;
        Switch1.ethg++ <--> C <--> LD1.ethg++;
        Switch1.ethg++ <--> C <--> CM1.ethg++;
        Switch1.ethg++ <--> C <--> US2.ethg++;
        Switch1.ethg++ <--> C <--> LD2.ethg++;
        Switch2.ethg++ <--> C <--> Switch1.ethg++;
        Switch2.ethg++ <--> C <--> RS1.ethg++;
        Switch2.ethg++ <--> C <--> RS2.ethg++;
        Switch2.ethg++ <--> C <--> S4.ethg++;
        Switch2.ethg++ <--> C <--> S3.ethg++;
        Switch2.ethg++ <--> C <--> US3.ethg++;
        Switch2.ethg++ <--> C <--> ME.ethg++;
        Switch2.ethg++ <--> C <--> RC.ethg++;
        Switch2.ethg++ <--> C <--> TLM.ethg++;
        Switch2.ethg++ <--> C <--> US4.ethg++;
}
