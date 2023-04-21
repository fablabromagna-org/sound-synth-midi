# MIDI BLE con Micropython e ESP32

Resoconto riassuntivo della sperimentazione fatta su ESP32 per implementare un controller MIDI BLE.


## Sperimentazione TIvan Aprile 2023

Dopo aver testato con successo un'implementazione MIDI-BLE su ESP32 con framework Arduino (su PlatformIO) ho voluto provare a replicare una cosa simile utilizzando il linguaggio MicroPython.

> L'idea inziale era quella di confrontare le implementazioni MicroPython e CircuitPyhthon, ma dopo le prime verifiche ho scoperto che allo stato attuale CircuitPython supporta solo i nordic nrf52, di cui non sono in possesso.
> Quindi al momento l'unica implementazione su cui ho potuto sperimentare è stata quella MicroPython (uPython)

Le schede ESP32 sono al momento la soluzione più economica per sperimentare una soluzione MIDI-BLE hanno il vataggio di essere supportate sia dal framework Arduino (in aggiunta a IDF) che da MicroPython.


### MIDI-BLE Abstract

L'implementazione testata prevede l'ESP32 nel ruolo di BLE periheral , mentre il central è tipicamente un PC o un tablet/smartphone. In questo modo ESP32 viene utilizzato come fosse un controller o una tastiera USB, da utilizzarsi in un sw su PC/Mobile come MIDI-IN.

L'implementazione MIDI su BLE prevede, come standard, di esporre un service con UUID "03b80e5a-ede8-4b33-a751-6ce34ec4c700".
All'interno di tale service va definita una characteristic con UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"; è attraverso quest'ultima che il dispositivo invierà messaggi MIDI (NOTIFY) ed eventualmente ne riceverà (WRITE).

> L'attuale implementazione non prevede che il device ESP32 riceva messaggi MIDI, ma per lo standard ho comunque definito anche il flag WRITE

Per essere riconosciuto da un BLE-Central (PC o mobile) come dispositivo MIDI, il dispositivo deve inviare nel pacchetto di advertising anche l'UUID del service MIDI.


### MIDI-BLE - Implementazione MicroPython con lib ubluetooth

Ho condotto i test utilizzando MicroPython v1.19.1 per ESP32, con IDE Thonny 4.0.2.

Per la parte di advertising (il messaggio BLE che viene inviato al boot in modo che il device sia rilevato) ho utilizzato un modulo trovato in rete, ed utilizzato in diversi progetti di esempio: `ble_advertising.py`. Questo facilita la cotruzione del messaggio BLE da inviare. Tale file, per poter essere importato dal programma principale, deve essere copiato sul filesystem del dispositivo.


Il codice di esempio è invece contenuto nel file `test_midi_ble.py` che vado brevemente a descrivere:

- viene definita una classe BLE che nel costruttore crea una istanza di `bluetooth.BLE()` e la attiva 
- inoltre viene collegato un metodo ble_irq agli eventi IRQ ricevuti dal BLE, in modo da processare gli eventi generati dalla lib (questa parte è un po' oscura nel funzionamento, ma l'ho trovata negli esempi)
- nella classe viene definito il metodo `register()` che si occupa di registrare Service e Characteristic con i relativi UUID e flag
- infine viene costruito il payload di advertising (tramite modulo esterno) , che verrà poi passato al metodo `ble.gap_advertise()` nel metodo della classe `advertiser()`
- nel main del programma viene quindi istanziata la classe passando un nome che dovrebbe poi identificare il device 
- viene avviata la registrazione del servizio e l'avvio dell'advertising e ci si mette in attesa di un collegamento da parte di un BLE-Central.
- a connessione avvenuta inizia un loop di invio di comandi MIDI NoteOn e NoteOff, in modo da poter provare il dispositivo anche in assenza di bottoni o sensori esterni



### Esito e considerazioni sui test eseguiti

Questo primo e semplice esempio funziona, anche se ci sono alcune criticità da sistemare e alcuni dubbi.

Ho provato questo codice sia su ESP32 che ESP32-S3. Ho utilizzato come BLE Central:
- un PC Linux
- un MacMini (con OSx piuttosto vecchio)
- uno smartphone Android
- un iPad Mini 2

Su tutte le piattaforme testate il device viene rilevato, collegato e i comandi MIDI vengono ricevuti correttamente.

Stranamente però ESP32 e ESP32-S3 si comportano diversamente in quanto sul primo, indipendenemente dal nome passato alla classe, via BLE compare sempre il nome ESP32. Stessa versione di micropython, ma ogni device ha una build apposita.

Inoltre impostando un nome più lungo di 4-5 caratteri sembra che la lib  vada in crisi durante l'advertising (da indagare - forse un bug o un limite nel buffer interno).

La gestione del flag di connessione avvenuta va gestito meglio per evitare l'accavallamento di eventi IRQ con il loop main.


Rispetto al dispositivo implementato con Arduino, questa versione ha 2 comportamenti problematici, in Android e Linux.

Sul primo, viene riconosciuto dall'app MIDI+BTLE ma al momento di eseguire la connessione mostra uno strana icona come di errore. Nonostante questo il successivo funzionamento è garantito. Attraverso l'app MIDI Scope posso vedere i messaggi.

Con Linux invece il primo abbinamento va a buon fine, vedo il device MIDI nelle app di routing Audio/MIDi di sistema e posso catturare i messaggi con MIDI Monitor. Quando però vado a spegnere e riaccendere il dispositivo, lo stack BT di Linux tenta il ricollegamento automatico che però fallisce, ed entra in un loop di connessione-disconnesisone infinito. 
Credo di aver individuato un problema a livello di autenticazione, ma devo indagare. Se si disaccoppia e si riesegue la connessione tutto torna ok (ma rente il device inutilizzabile in un caso reale).
> l'implementazione Arduino funziona invece correttamente anche in questo frangente con Linux.

Su iPad e OsX il problema non si pone perchè (almeno nelle versioni non recenti da me testate, il device deve essere riconnesso manualmente ogni volta).


### Prossimi step

- approfondire le problematiche del nome nell'advertising, analizzando la lib ubluetooth
- approfondire la problematica di riconnessione su Linux
- testare la libreria di più alto livello di micropython, che nella doc consigliano rispetto a questa (aioble)
- fare qualche test con sensori/pulsanti esterni, per verificare prestazioni e latenza
