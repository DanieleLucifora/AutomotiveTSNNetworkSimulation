# Industrial & Automotive Real-Time Networks – OMNeT++ Simulation

Questo progetto è un elaborato del corso di **Industrial and Automotive Real-Time Networks** e consiste nella simulazione, tramite OMNeT++, di una rete **Switched Ethernet TSN (Time-Sensitive Networking)** per applicazioni automotive con traffico cross-domain.

## Scenario simulato

La rete simulata:
- Comprende **18 end-node** e **2 switch**
- Trasporta traffico appartenente a due domini: **ADAS** (Advanced Driver Assistance Systems) e **Infotainment**
- È caratterizzata da un data rate di **100 Mbps**

### Tecnologie utilizzate
- **OMNeT++** come framework di simulazione
- **INET framework** per la modellazione di protocolli TCP/IP e switch Ethernet
- **Schedulazione**: Strict Priority (SP) e Rate Monotonic (RM)

## Architettura

Ogni nodo è un `TsnDevice`, estensione del modulo `StandardHost`, e implementa:
- Uno o più livelli applicativi (`EthernetApplication`) per generare/ricevere i flussi
- Code di trasmissione con priorità da 0 a 7
- Algoritmo di selezione SP per l’invio dei frame

Ogni switch è un modulo `TsnSwitch`, estensione di `EthernetSwitch`, configurato per supportare la **Qualità del Servizio (QoS)**.

## Flussi e traffico

| Flusso        | Periodo | Payload  | Burst | Priorità |
|---------------|---------|----------|--------|----------|
| Lidar → CU    | 1.4 ms  | 1300 B   | 1      | 5        |
| Camera F. → HU| 16.66 ms| 1500 B   | 119    | 3        |
| RC → HU       | 33.33 ms| 1500 B   | 119    | 3        |
| ME → RS*      | 33.33 ms| 1500 B   | 119    | 2        |
| ME → S*       | 250 µs  | 80 B     | 1      | 7        |
| US → CU       | 100 ms  | 188 B    | 1      | 1        |

> Tutte le priorità sono assegnate tramite algoritmo Rate Monotonic.

## Metriche valutate

- **End-to-End Delay**
- **Absolute Jitter**
- **Frame Loss Ratio (FLR)**

I risultati sono raccolti al variare del workload e delle fasi di trasmissione, osservando:
- Saturazione delle code nei nodi e switch
- Impatto della frequenza dei frame sui flussi video (ME e CM1)
- Effetti del traffico su delay e perdita pacchetti

## Risultati principali

- Il nodo **ME** genera un traffico di ~94.78 Mbps → satura la propria coda di uscita
- La **porta dello Switch1** collegata a **HU** va in saturazione → FLR elevato per i flussi video
- Riduzione della frequenza dei frame (da 30fps a 24fps) riduce il carico → migliora FLR e jitter

## Contenuto della repository

- `/simulations/`: file `.ned`, `.ini` e `.xml` per la simulazione
- `/src/`: moduli customizzati per `TsnDevice` e `TsnSwitch`
- `/results/`: risultati delle metriche e grafici (.vec, .sca, .png)

## Autore

Daniele Lucifora  
Corso di Laurea Magistrale in Ingegneria Informatica  
Università degli Studi di Catania – 2025

---

Per domande o collaborazioni, contattami su [LinkedIn](https://www.linkedin.com/in/danielelucifora/)
