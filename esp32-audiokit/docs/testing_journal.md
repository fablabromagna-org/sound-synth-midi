# Diario deile attività di sperimentazione e test su scheda ESP32-AudioKit e relative librerie


## Fix libreria ML_SynthTools - 10/08/2023

In seguito ai continui errori di inizializzazione del codec ho deciso di aprire una issue sul progetto della lib ML_SynthTools:
https://github.com/marcel-licence/ML_SynthTools/issues/35

Sono stato contatto velocemente da Marcel che mi ha dato qualche indicazione su come eseguire il debug. A seguito di questi approfondimenti ho trovato un errore nella gestione della comunicazione i2c e ho suggerito la soluzione a Marcel, il quale ha prontamente corretto il problema (il bello dell'open source!!).

Ora quindi gli esempi della lib funzionano tutti senza errori.

Ho inoltre provato a compilare il progetto [esp32_midi_sampler](https://github.com/marcel-licence/esp32_midi_sampler), che dai video sembra molto interessante. Sembra funzonare correttamente, ma per poterlo testare a fondo in tutte le funzionalità occorre gestire una serie di comandi MIDI CC che al momento non sono in grado di generare facilmente.

Devo quindi provare con la MK più grossa, dotata di pulsanti aggiuntivi. In alternativa stavo pensado di implementare una semplice applicazione Python (magari con kivy) per generare una sorta di pulsantiera sw virtuale (ispirata al Novation Launchpad) che permetta di abbinare un CC ad ogni pulsante, e inviarlo poi attraverso ttymidi.

Al momento ho sospeso i test per completare il progetto MiniDex che stavo montando usando un Raspberry Pi Zero 2. Tornerò quindi sul pezzo nei prossimi giorni.



## MIDI seriale e USB Host con Raspberry Pi Pico - 2/08/2023

Per riuscire a comandare via MIDI la scheda ESP32-Audiokit, e poter così testare la libreria ML_SynthTools, ho approntato 2 soluzioni:

La prima prevede di usare un semplice convertitore USB-TTL collegato ad un PC (nel mio caso GNU/Linux). GND e TX in uscita dal convertitore vengono poi collegati a GND e RX-GPIO della scheda. Sul PC viene poi eseguita una utility chiamata `ttymidi` che crea sul sistema una porta MIDI virtuale. Occorre passare il device USB e il baud: `ttymidi -b115200 -v -s /dev/ttyUSB1`.
A questo punto la nuova porta MIDI diventa visibile sul sistema e attraverso JACK o PipeWire e relative app di routing è ad esempio possibile utilizzare una tastiera MIDI virtuale (VMPK Virtual Piano) e reindirizzarne i messaggi MIDI verso la seriale USB. Quindi suonando con la tastiera MIDI virtuale i relativi messaggi di note-on e note-off arrivano alla scheda. L'unica accortezza è quella di ridefinire, nel codice degli esempi, il baudrate del MIDI, che di default è 31250 (un valore non gestibile da PC). In questo modo, attivando il debug MIDI sugli esempi, mi è stato possibile verificare il corretto funzionamento. Gli esempi suonano ma continuano a mostrare una serie di errori relativi all'inizializzazione del CODEC.


La seconda soluzionè MIDI è invece stata quella di usare un Raspberry Pi PICO (nel mio caso un Pico Zero) e attraverso il progetto ` https://github.com/rppicomidi/midi2usbhost.git` trasformarlo in un convertitore USB-Host <-> MIDI seriale.

In pratica, una volta compilato e scaricato sul Pico il firmware del progetto, la sua USB diventa una USB-Host a cui poter collegare una master keyboard USB (senza dover quindi implementare il circuito convertitore, che tra l'altro prevederebbe che la MK venisse alimentata esternamente).
Una volta collegata ma MK e negoziato il protocollo USB, i messaggi MIDI ricevuto dalla MK vengono inoltrati su un PIN GPIO usato come serial TX, al baud rate standard 31250.
Ho potuto verificato oscilloscopio alla mano la corretta conversione al baud specificato.

L'unico inconveniente di questa soluzione è che occorre una alimentazione esterna 5V per il PICO, visto che la sua unica seriale viene ora usata come USB Host. Tale alimentazione va quindi applicata sugli appositi pin GPIO, e serve inoltre ad alimentare la MK via USB.


## Getting started - 31/07/2023

Dopo l'unboxing ho tentato di testare la scheda con gli esempi dell'[ESP-ADF](https://docs.espressif.com/projects/esp-adf), nella speranza che fosse il punto più semplice e più ideneo da cui partire.

Dopo aver seguito le istruzioni per l'installazione di IDF e ADF sono stato in grado di compilare alcuni esempi ADF. Ma una volta scaricati sulla scheda non hanno prodotto alcun audio in uscita.
Ho cercato di indagare sulle possibili cause, per capire se poteva trattarsi di qualcosa a livello di configurazione, ma il framework è piuttosto complicato e gli esempi forniti sono tutt'altro che semplici da capire/modificare. Non sembra infatti esserci il classico esempio basilare con cui produrre semplicemente un tono in uscita. Ho quindi deciso di abbandonare momentaneamente questa strada.


----------

Sono passato poi a provare alcuni esempi della libreria [Arduino Audio Tools](https://github.com/pschatzmann/arduino-audio-tools) con Arduino IDE 2.
Ho per prima cosa installato manualmente da github (tramite `git clone`) la libreria in questione e la libreria [Arduino ADF/AudioKit HAL](https://github.com/pschatzmann/arduino-audiokit), che fornisce il supporto della scheda ESP32-Audiokit alla prima librerie.

Alcuni degli esempi che ho provato (presi direttamente dagli esempi della lib) sono riportati nella sottodirectory `examples/audio-tools`.

In questo caso le cose sono andate subito molto bene. Dopo aver configurato opportunamente la scheda, gli esempi hanno subito dato risultati positivi.
Gli esempi già indirizzati alla scheda esp32-audiokit funzionano senza modifiche, mentre per gli altri generici occorre semplicemente ridefinire l'istanza del dac i2s usando la classe AudioKitStream  :  `AudioKitStream i2s;  // Output to AudioKit`

L'unica "scocciatura" iniziale è la necessità di installare manualmente tutta una serie di librerie aggiuntive, dipendenze necessarie al funzionamento della lib principale. Questo in parte l'ho fatto da LibraryManager e in parte manualmente da github.


Tra gli esempi interessati:

`streams_synthstk_audiokit.ino` mi ha permesso di generare una serie di note alla pressione dei tasti a bordo della scheda.
`streams-memory_mp3-audiokit.ino` permette di riprodurre un file mp3
`streams-memory_raw-i2s.ino` riproduce la sigla di starwars , contenuta nel file .h
`basic_a2dp_audiokit` permette di trasformare la scheda in uno speaker bluetooth, rendendo il device disponibile al discovery e riproducendo lo stream proveniente dallo smartphone; il tutto in meno di 40 righe di codice !!!
`ble_receive.ino` implementa infine un client MIDI BLE in 35 righe di codice, permettendo di ricevere messaggi MIDI da PC, tablet o smartphone.

La libreria è molto vasta e ci sono tantissimi esempi; i risultati sono molto incoraggianti quindi merita sicuramente un approfondimento maggiore e più metodico.

----------

Infine sono passato a testare la libreria [ML_SynthTools - Arduino synthesizer library](https://github.com/marcel-licence/ML_SynthTools)

Anche in questo caso ho installato manualmente la libreria da github. Occorre poi modificare il file `src/boards/board_audio_kit_es8388.h` all'interno della lib, in modo da specificare la corretta configurazione dei pin del CODEC (i2c e i2s), dato che esistono versioni di scheda diverse con pin usati differenti.
Nel mio caso ho usato:
```
#define ES8388_CFG_I2C  2
#define ES8388_CFG_I2S  5
```

Dopo aver compilato alcuni esempi ho però ottenuto una serie di errori a runtime nell'inizializzazione del codec e non sono riuscito a testare il corretto funzionamento. Va anche detto che gli esempi allegati alla lib presuppongono di ricevere MIDI sulla seriale (pin GPIO RX) e questo presuppone avere una masterkeyboard e relativo circuito di conversione da MIDI DIN a MIDI seriale TTL 3v3. Non essendone provvisto ho temporaneamente interrotto i test.
