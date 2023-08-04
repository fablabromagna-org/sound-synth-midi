# Effetto echo realizzato con Raspberry Pi Pico 
### Un esercizio per illustrare alcuni concetti dell'elaborazione audio real-time

Questo semplice dispositivo realizzato con un Raspberry Pi Pico ed una coppia di DAC I2S MAX98357A, è un effetto echo stereo (anche detto delay) con campionamento 12bit - 40Ksps (samples per second); non include alcuna regolazione esterna né filtri anti aliasing agli ingressi, perchè lo scopo è puramente didattico.

Al Pi Pico è demandato il ruolo di effettuare la lettura/campionamento dei segnali in ingresso (left - right) utilizzando il convertitore ADC 12bit interno alla board, quindi eseguire le elaborazioni necessarie (scrivere nelle code FIFO i sample letti, leggere sample dalle code FIFO, effettuare moltiplicazioni e somme), infine inviare un flusso audio digitale 16bit - 40Ksps a ciascuno dei due DAC, utilizzando connessioni fisiche a 3 fili con protocollo I2S.

A loro volta i DAC effettuano una conversione da digitale a PWM, con frequenza 300Khz, utilizzando in uscita un circuito a bassa impedenza d'uscita ed elevata corrente, in grado quindi di pilotare direttamente gli altoparlanti 4-8Ω; le caratteristiche di impedenza e risposta elettromeccanica degli altoparlanti abbattono e rendono inudibili le componenti audio del flusso PWM superiori 15-20kHz.

Nel video seguente una breve registrazione dell'effetto; all'ingresso è collegato un generatore di funzioni con sweep regolato manualmente:

https://github.com/fablabromagna-org/sound-synth-midi/assets/41198542/2fde0d05-7ad2-4d5e-9d04-8c904a834ee6


#### Schema a blocchi

Lo schema ingresso-uscita seguente illustra l'articolazione dell'effetto echo nei suoi componenti: il delay ed i blocchi sommatore e moltiplicatore , l'interconnessione tra gli stessi e la direzione del flusso di elaborazione.

<p align="left">
<img width="600" src="/pi_pico_echo_stereo/media/delay_0.jpg")
</p>

Lo schema nasce da una semplice intuizione: per esperienza comune un suono con eco è dato dalla somma (in aria) del suono origine, dello stesso suono riflesso (che giunge con un certo ritardo all'origine), e delle ulteriori riflessioni delle riflessioni.

Nel passaggio dallo schema a blocchi al codice occorre tener presente che il segnale trattato non è continuo ma campionato; non a tempo continuo ma a tempo discreto. Questo significa che occorre elaborare singoli campioni e fornire in uscita, analogamente, singoli campioni.

Nell'esercizio, il codice ha la totale responsabilità del rispetto del timing. La funzione "repeating_timer_callback", che viene chiamata via interrupt ogni 25us (tempo di campionamento), svolge la funzione di leggere (campionare) i due segnali in ingresso (utilizzando i convertitori AD interni alla scheda Raspberry Pi Pico), effettuyare le elaborazioni audio richieste, infine inviare due campioni (sample) ai due convertitori DA esterni. 25us rappresenta il tempo concesso per effettuare l'elaborazione audio desiderata, e corrisponde ad una frequenza di campionamento pari a 40ksps.

In termini di codice, lo schema a blocchi si traduce in una singola funzione lineare; infatti se:
- D è il valore del ritardo (come numero di sample) introdotto dal blocco delay
- x(t) il sample in ingresso al tempo t
- y(t) il sample in uscita al tempo t

dallo schema a blocchi risuta che:
Y(t) = C*X(t) + K*Y(t-D)

La presenza dei parametri C e K è prima di tutto dovuta al fatto che la lunghezza di parola è finita (16bit), ed essi aiutano a prevenire fenomeni di saturazione numerica. Il parametro K ha una ulteriore importante ricaduta, perchè da esso dipende la "stabilità" dell'algoritmo: per valori K > 1 il calcolo diverge rapidamente in presenza del più piccolo e breve segnale in ingresso. Tratteremo il tema della stabilità nelle sezioni successive introducendo ulteriori semplici strumenti di analisi.

Il blocco delay è realizzato con un array di dimensione almeno pari a D+1; nel codice si usano due array, uno per canale, di dimensione 14000, in grado di memorizzare 14000 sample, pari a 14000*25us=350ms; il valore massimo per D è quindi 13999*25us.


#### Modellizzazione

L'algoritmo riceve in input una successione di valori (sample) misurati dell'ingresso e fornisce in output un'altra successione di valori:

<p align="left">
<img width="600" src="/pi_pico_echo_stereo/media/z_0.jpg")
</p>

Chiamiamo in(n) la sequenza di sample in ingresso, out(n) la sequenza di sample in uscita, con n = 0, 1, 2, etc.
In quale relazione stanno le due sequenze? Per verificarlo senza ricorre a modelli matematici occorre costruire materialmente il dispositivo, scriverne e compilarne il codice, definire un banco di prova con generatore di funzioni e oscilloscopio per la visualizzazione dei due segnali. In alternativa si crea un _modello matematico_, e lo si studia con simulazioni automatiche.


## Modello per la serie di campioni
Per descrive una sequenza di campioni in termini matematici, partiamo da una sequenza detta "impulso unitario" δ(n), così definita:
       
_δ(n) vale +1 se n=0 , vale 0 altrove._

La sequenza ha il primo campione di valore +1, tutti i campioni seguenti con valore 0. La sequenza seguente:
    
_δ(n-P) vale +1 se n-P=0 (cioè n=P) , vale 0 altrove._

Siamo ora in grado di descrivere una qualsiasi sequenza di campioni y(n); se scriviamo:

_y(n) = k0*δ(n) + k1*δ(n-1) + k2*δ(n-2) + ......_

La sequenza y(n) ha il primo sample di valore k0, il secondo di valore k1, etc.

Eseguiamo ora la seguente applicazione (trasformazione) sulla sequenza di campioni y(n), che chiamiamo _trasformata Z_ di y(n); definiamo la funzione trasformata Y(z) seguente:

_Y(z) = y(0)*z^0 + y(1)*z-1 + y(1)*z-2 + ..... _ 

e ricordando che per qualsiasi valore z si ha z^0 = 1:

_Y(z) = y(0) + y(1)*z-1 + y(1)*z-2 + ..... _ 

Si noti che se la trasformata Z di δ(n) vale 1; infatti in questo caso:
_Δ(z) = δ(0) + 0 + 0 + .... = 1_

Esistono altre particolari sequenze per cui la funzione Z trasformata assume forme di rilievo; ad esempio, si può dimostrare che la sequenza costituita da sample unitari, ha la seguente trasformata Z:

_G(z) = 1 + z^-1 + z^-2 + z^-3 + ........ = z/(z-1)_


## Modello per l'algoritmo