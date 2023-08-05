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
- x(n) il sample in ingresso all'istante n
- y(n) il sample in uscita all'istante n

dallo schema a blocchi risuta che:
_y(n) = C*x(n) + K*y(n-D)_

La presenza dei parametri C e K è prima di tutto dovuta al fatto che la lunghezza di parola è finita (16bit), ed essi aiutano a prevenire fenomeni di saturazione numerica. Il parametro K ha una ulteriore importante ricaduta, perchè da esso dipende la "stabilità" dell'algoritmo: per valori K > 1 il calcolo diverge rapidamente in presenza del più piccolo e breve segnale in ingresso. Tratteremo il tema della stabilità nelle sezioni successive introducendo ulteriori semplici strumenti di analisi.

Il blocco delay è realizzato con un array di dimensione almeno pari a D+1; nel codice si usano due array, uno per canale, di dimensione 14000, in grado di memorizzare 14000 sample, pari a 14000*25us=350ms; il valore massimo per D è quindi 13999*25us.


#### Modellizzazione

L'algoritmo riceve in input una successione di valori (sample) misurati dell'ingresso e fornisce in output un'altra successione di valori:

<p align="left">
<img width="600" src="/pi_pico_echo_stereo/media/z_0.jpg")
</p>

Chiamiamo in(n) la sequenza di sample in ingresso, out(n) la sequenza di sample in uscita, con n = 0, 1, 2, etc.
In quale relazione stanno le due sequenze? Per verificarlo senza ricorre a modelli matematici occorre costruire materialmente il dispositivo, scriverne e compilarne il codice, definire un banco di prova con generatore di funzioni e oscilloscopio per la visualizzazione dei due segnali. In alternativa si crea un _modello matematico_, e lo si studia con simulazioni automatiche.


##### Modello per sequenze/serie di campioni
Per descrive una sequenza di campioni in termini matematici, partiamo da una sequenza detta "impulso unitario" δ(n), così definita:
       
_δ(n) vale +1 se n=0 , vale 0 altrove._

La sequenza ha il primo campione di valore +1, tutti i campioni seguenti con valore 0. La sequenza seguente:
    
_δ(n-P) vale +1 se n-P=0 (cioè n=P) , vale 0 altrove._

Siamo ora in grado di descrivere una qualsiasi sequenza di campioni y(n); se scriviamo:

_y(n) = k0*δ(n) + k1*δ(n-1) + k2*δ(n-2) + ......_

la sequenza y(n) ha il primo sample di valore k0, il secondo di valore k1, etc.

Eseguiamo ora la seguente applicazione (trasformazione) sulla sequenza di campioni y(n), che chiamiamo _trasformata Z_ di y(n); definiamo la funzione trasformata Y(z) seguente:

_Y(z) = k0*z^0 + k1*z^(-1) + k2*z^(-2) + ....._ 

e ricordando che per qualsiasi valore z si ha z^0 = 1:

_Y(z) = k0 + k1*z^(-1) + k2*z^(-2) + ....._ 

Si noti che la trasformata Z di δ(n) vale 1; infatti in questo caso:

_Δ(z) = 1 + 0*z^(-1) + 0*z^(-2) + .... = 1_

Infine, se consideriamo la generica sequenza r(n) ottenuta ritardando la serie y(n) di D campioni:

_r(n) = 0*δ(n) + 0*δ(n-1) + 0*δ(n-2) + ......+ k0*δ(n-D) + k1*δ(n-D-1) + k2*δ(n-D-2) + ......_

Trasformando r(n), otteniamo:

_R(z) = k0*z^(n-D) + k1*z^(n-D-1) + k2*z^(n-D-2) + ..... = z^(-D) * (k0*δ(n) + k1*δ(n-1) + k2*δ(n-2) + ......) = z^(-D) * Y(Z)

Cioé: la trasformata Z di una successione y(n) ritardata di D campioni si ottiene moltiplicando la trasformata di y(n) per z^(-D).
Infine, altra importante proprietà della trasformata Z: date due successioni, x(n) ed y(n) e la successione somma s(n)=x(n)+y(n), la trasformata Z di s(n) vale:

_S(z) = X(z) + Y(z)_




##### Modello per l'echo
Abbiamo visto la relazione ingresso uscita dell'algoritmo per l'echo:

_y(n) = C*x(n) + K*y(n-D)_

Applicando la trasformata Z a questa uguaglianza otteniamo:

_Y(z) = C*X(z) + K*z^(-D)*Y(z)_

da cui:

_Y(z) = X(z) * C/(1 - K*z^(-D))_

Definiamo il secondo termine di questa moltiplicazione funzione di trasferimento del nostro echo:

_H(z) = C/(1 - K*z^(-D))_

da cui:

_Y(z) = X(z) * H(z)_

Attraverso questa serie di passaggi, abbiamo ridotto una elaborazione nel dominio del tempo in una semplice moltiplicazione in un nuovo dominio (quello della variabile z) che, vedremo è legato alla frequenza.


