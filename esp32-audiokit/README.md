# ESP32-AudioKit

Resoconto riassuntivo della sperimentazione fatta su ESP32-AudioKit, con diversi framework e librerie.

_(Ivan Tarozzi - Luglio 2023)_


## La scheda ESP32-AudioKit

Progettata dall'azienda cinese A.I. Thinker è basata su un ESP32-A1S, ovvero un modulo ESP32 con integrato un CODEC i2s.

Esistono diverse versioni del chip (e di conseguenza della scheda) purtroppo non semplici da distinguere e non perfettamente compatibili le une con le altre al 100%.

In particolare c'è stato nel tempo un cambio del CODEC i2s passando da un AC101 ad un ES8388.

Non è inoltre chiaro se tutte le schede basate su ES8388 utilizzino gli stessi pin per il controllo del CODEC via i2c e i2s.

Nella cartella doc è presente il datasheet della scheda: [esp32-a1s_v2.3_specification.pdf](docs/esp32-a1s_v2.3_specification.pdf)

-----------

La scheda ha alcune caratteristiche che la rendono particolarmente interessante per sperimentare e farsi un'idea delle possibilità offerte da ESP32 in ambito audio, e più in generale per testare i framework audio disponibili anche per arduino, e quindi anche per altri MCU (RP2040, Teensy, ...).  Ad esempio:

- uscita cuffie stereo preamplificata
- uscita speaker stereo amplificata
- ingresso LINE-IN stereo
- 2 microfoni sul PCB
- Slot SD Card
- connettore batteria LiPo
- alcuni pulsanti sul PCB
- un paio di led su PCB


Di seguito invece alcune "mancanze" emerse durante la sperimentazione (da verificare se sono aggirabili approfondendo ulteriormente lo studio):

- mancanza di un connettore MIDI-IN e/o MIDI-OUT DIN
- mancanza di connessione MIDI USB Host (per connettere direttamente, ad esempio, una master keyboard USB)
- mancanza di un LCD integrato (facilmente aggirabile utilizzando display esterni SPI, ma occorre capire se coesistenti con la SD - oppure I2C)
- numero limitato di GPIO, aggirabile eventualmente con expander i2c
- mancanza di un pin di uscita 5V con cui alimentare altre schede
- mancanza di pin di alimentazione alternativo, oltre alla porta micro-USB



## SDK, Librerie e Software

Come avviene per le classiche schede basate su ESP32 (e varianti), la programmazione può avvenire utilizzando diversi framework.

### ESP-IDF e ESP-ADF
Il più completo è senz'altro [ESP-IDF](https://idf.espressif.com) di Espressif, di fatto l'SDK ufficiale del creatore dei chip.

Inoltre, per la parte audio, Espressif mette a disposizione [ESP-ADF](https://docs.espressif.com/projects/esp-adf), un framework dedicato allo sviluppo audio su ESP32 e varianti. ADF si appoggia a IDF, estendendolo con una serie di librerie per facilitare lo sviluppo di applicazioni audio, quali:
- gestione dei codec audio I2S
- gestione di storage (es SD) con audio files
- gestione di stream audio via rete (wifi) o bluetooth

ESP-ADF utilizza come board di riferimento la ESP32-LyraT, che monta il codec ES8388. Essendo lo stesso della ESP32-AudioKit questo dovrebbe favorire l'esecuzione di esempi ADF.


### Librerie Arduino di Phil Schatzmann: arduino-audio-tools & Co.

[Phil Schatzmann](https://www.pschatzmann.ch) ha creato una vasta serie di librerie per framework Arduino che supportano una serie di MCU tra cui anche ESP32.

Le librerie più interessanti sono:

- [Arduino Audio Tools](https://github.com/pschatzmann/arduino-audio-tools)
- [Arduino ADF/AudioKit HAL](https://github.com/pschatzmann/arduino-audiokit)
- [Arduino Midi](https://github.com/pschatzmann/arduino-midi)
- [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP)
- [Synthesis ToolKit (SKT)](https://www.pschatzmann.ch/home/projects/the-synthesis-toolkit-skt-library-for-arduino/)


In particolare __arduino-audio-tools__ mette a disposizione una quantità notevole di librerie ed esempi per l'audio processing, spaziando da audio encoding/decoding (MP3, ACC, WAV, FLAC) a Sound Generator & Effects, da Audio Streaming ad Audio Sinks di vario tipo (DAC I2S, PWM, Eq, LED Strip/Matrix, FFT, ...).

Per poter utilizzare la libreria __arduino-audio-tools__ con ESP32-AudioKit e LyraT, ci si appoggia sul repository __arduino-audiokit__, che fornisce un layer aggiuntivo per la configurazione e l'utilizzo dei CODEC e della SD via SPI.

Da notare inoltre che la scheda ESP32-AudioKit è spesso utilizzata da Schatzmann come board di riferimento e negli esempi di __arduino-audio-tools__ una intera sezione è dedicata ad essa :
[examples/examples-audiokit](https://github.com/pschatzmann/arduino-audio-tools/tree/main/examples/examples-audiokit)


Non ho però finora trovato progetti di synth completi, basati su tali librerie, seppure la rete sia piena di tutorial che si focalizzano spesso sulle singole funzionalità... magari è solo questione di cercare meglio su github


### Librerie Arduino di Marcel License

[Marcel License](https://github.com/marcel-licence) ha creato una libreria Arduino per la sintesi musicale, più una serie di progetti che si appoggiano ad essa per la creazione di organi e pianoforti elettronici, drum machine, multitrack looper. La maggior parte di questi sono basati su ESP32 e scheda AudioKit.

Le librerie e i progetti più interessanti sono:

- [ML_SynthTools - Arduino synthesizer library](https://github.com/marcel-licence/ML_SynthTools)
- [ESP32 based DIY polyphonic MIDI synthesizer](https://github.com/marcel-licence/esp32_basic_synth)
- [ESP32 Audio Kit based multitrack looper](https://github.com/marcel-licence/esp32_multitrack_looper)
- [ESP32 Audio Kit Sampling MIDI Module](https://github.com/marcel-licence/esp32_midi_sampler)

La maggior parte dei progetti di Marcel sono corredati da video YouTube che mostrano i risultati che si riescono ad ottenere.





## Sperimentazione, test e progetti

Dopo l'introduzione sulla scheda ESP32-AudioKit e la carrellata sui possibili framework da utilizzare, seguono una serie di articoli riguardanti esperienze personali di utilizzo, difficoltà incontrate, soluzioni adottate. Il tutto per fare il punto sulle possibilità di sviluppo, sulla maturità dei framework e sulla effettiva facilità di programmazione, che non sempre è quella dichiarata sulla carta.

- [Diario dei test](docs/testing_journal.md)
