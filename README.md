# Audio, Synth & MIDI

Un repository per raccogliere idee, informazioni e progetti riguardanti la sintesi sonora, i controller MIDI e tutto quello che, in ottica Maker, gira attorno al mondo della musica.

Info, link dei vari progetti e sistemi, o dei componenti elettronici utilizzati, verranno elencati nei successivi paragrafi.

Eventuali esempi di codice saranno inseriti nella directory `/sources` mentre progetti piÙ completi troveranno spazio in un repository dedicato, di cui inserire il link nell'apposita sezione di questo documento.



## I progetti dei nostri makers

- [Lilla](http://lillasampler.it) - Sandro Grassia
- [MiniDexed](https://github.com/probonopd/MiniDexed) - Simone Tomaselli
- [LOFI POPS 7](https://www.youtube.com/watch?v=sYu-AwnDNEU&t=2s)- Simone Tomaselli


## Tecnologie, progetti e librerie 

In ordine sparso le tecnologie SW adottate nei progetti relativi all'audio/MIDI:

- [CircuitPython](https://circuitpython.org/)
    - [libreria MIDI USB](https://docs.circuitpython.org/en/latest/shared-bindings/usb_midi/index.html)
    - [libreria MIDI](https://docs.circuitpython.org/projects/midi/en/latest/api.html)

- [Thonny Python IDE](https://thonny.org/)

## Synth software e tools per PC/mobile

- [HELM](https://tytel.org/helm/) - Free software synthesizer (GPL). 



## Componenti HW

- 

----
## Synth@Makerspace


### Serata danzante 30/3/2023
Si è svolta presso il makerspace si Santarcangelo una prima presentazione dei progetti realizzati dai nostri makers.

#### USB MIDI Controller 

Dopo una introduzione a base di cassoni santarcangiolesi offerti da FablabRomagna ai partecipanti, ha aperto le danze il nostro presidente  **Maurizio Conti** presentando un controller MIDI realizzato attorno ad un **Raspberry Pi Pico** (RP2040) con linguaggio **CircuitPython**.

In meno di 60 righe di codice il programma realizza un controller MIDI over USB, gestisce un encoder rotativo per l'invio di un comando MIDI CC e gestisce un modulo [Adafruit Trellis](https://learn.adafruit.com/adafruit-trellis-diy-open-source-led-keypad/overview) per l'invio di comandi MIDI NoteOn/NoteOff.

Per la gestione dell'HW, dei comandi MIDI e per l'implementazione dello stack MIDI over USB sono state utilizzare le librerie di Adafruit.

Come IDE per CircuitPython è stato mostrato **[Thonny](https://thonny.org/)**, un IDE piuttosto basico ma con caratteristiche interessanti tra cui:
- riconoscimento del device ed integrazione del REPL eseguito sul dispositivo
- salvataggio/caricamento di files sul "disco virtuale" del dispositivo 
- esecuzione e debug dei programmi micropython/circuitpython
- console seriale 

Per la dimostrazione pratica del funzionamento del controller è stato utilizzato un synth su PC di nome  **[HELM](https://tytel.org/helm/)**, un software multipiattaforma libero e open source.


#### Lilla

La palla è poi passata a **Sandro Grassia**, che ci ha presentato la sua creatura: **Lilla**. Si tratta di un audio sampler realizzato attorno ad un microcontrollore **Teensy**. Potete leggere la storia del progetto, che Sandro porta avanti da 5 anni, a questo [link](https://fablabromagna.org/lilla-story/).
Sandro ci ha descritto le macro funzionalità del suo sampler, la logica di funzionamento e alcuni dettagli implementativi specifici, a cui è seguita una demo audio e della navigabilità delle funzionalità tramite display integrato.
Infine ci ha mostrato un interessante [tool](https://www.pjrc.com/teensy/gui/index.html) su una interfaccia web che permette, combinando in modo visuale una serie di blocchi funzionali , di ottenere l'ossatura del codice C (con relativa istanza delle librerie teensy) da cui partire per sviluppare il firmware.

Sandro ha inoltre progettato  il contenitore del suo dispositivo con Fusion360 per poi realizzato  con taglio e piegatura di lamiera presso un'azienda locale.

Al questo link potete trovare maggiori **[informazioni sul progetto Lilla](http://lillasampler.it)**


#### Lofi Pops 7 & MiniDexed

Ha infine chiuso le danze **Simone Tomaselli** che ci ha presentato due progetti di sintesi sonora.

Il primo (**LOFI POPS 7**) realizza una riedizione in chiave moderna di un Kork Minipops anni 60. Realizzato su **Arduino UNO**, il dispositivo implementa una batteria elettronica con audio generato in PWM; è possibile selezionare diversi pattern ritmici preimpostati, variare volume e tempo e disabilitare selettivamente uno degli 8 "strumenti" che compongono il pattern. Inoltre è possibile utilizzare un segnale sync e reset esterno. 

Il secondo progetto (**MiniDexed**) invece implementa un emulatore software di una tastiera **Yamaha DX7** ed è basato su **Raspberry Pi**. La peculiarità di questa soluzione, rispetto ad altre implementazioni software per PC, è quella di utilizzare un sw baremetal sul Raspberry. Non viene quindi utilizzato alcun sistema operativo, ma viene eseguito solo il software di sintesi sonora. Oltre alla parte synth il software gestisce un display LCD, un encoder/pulsante e una porta MIDI IN attraverso il GPIO del Raspberry PI.
Il progetto open source che Simone utilizza nel suo prodotto è [MiniDexed](https://github.com/probonopd/MiniDexed).
> Nella pagina github è possibile trovare interessanti informazioni sull'utilizzo di sw baremetal su raspberry, sulle librerie di sintesi utilizzate e sui dettagli implementativi.

Per entrambi i progetti Simone utilizza scatole standard commerciali per le quali poi crea e disegna frontalini originali e customizzati anche sulle richieste dei clienti.

Potete trovare maggiori informazioni a questo [link](https://constructure.bio.link)


