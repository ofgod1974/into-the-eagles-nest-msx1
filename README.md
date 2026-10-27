# Into the Eagle's Nest — MSX1 / Fusion-C

Questa versione sostituisce la grafica schematica con fondali e mappe estratti dall'atlante C64 e fotogrammi dei personaggi decodificati dalla versione Amiga. E un port in sviluppo: le quattro piante sono esplorabili, ma le missioni complete dell'originale non sono ancora implementate.

## Compilazione da un clone GitHub

Il repository contiene sorgenti C/Z80, risorse, script di conversione, libreria Fusion-C e configurazione VS Code. Gli eseguibili di editor, compilatore ed emulatore, i file temporanei e le registrazioni audio restano esclusi da Git.

Su Windows servono **PowerShell 7** e **Node.js 20 o successivo**. Dalla cartella del repository:

```powershell
node scripts/setup-toolchain.cjs
./scripts/build.ps1
```

Il bootstrap scarica SDCC da una revisione fissata del bundle MSXgl e verifica i checksum SHA-256 dei 48 file. La ROM compilata viene scritta in `build/eagle.rom`. Gli header generati sono gia versionati: non e necessario riestrarre le risorse per compilare.

Per eseguire localmente, usare openMSX con macchina **C-BIOS_MSX1_EU**, ROM di tipo **Normal** e, facoltativamente, una seconda cartuccia SCC. I lanciatori inclusi cercano l'emulatore in `tools/openmsx`; sul PC di sviluppo e gia installato. Fonti ufficiali: [openMSX](https://openmsx.org/) e [C-BIOS](https://github.com/openMSX/openMSX/tree/master/Contrib/cbios). Il gioco si puo anche provare tramite il link online riportato alla fine di questo documento, accedendo con l'account proprietario.

Aprire la cartella con Visual Studio Code; **Ctrl+Shift+B** compila. Il comando `Apri progetto.cmd` usa l'editor portatile quando presente e altrimenti quello installato nel sistema.

## Gioca e modifica

- `Gioca.cmd`: avvia la ROM attuale su openMSX / C-BIOS MSX1 PAL.
- `Apri progetto.cmd`: apre il progetto nella copia portatile di Visual Studio Code inclusa.
- In VS Code: **Ctrl+Shift+B** compila; **Terminale > Esegui attivita > MSX1: Run openMSX** compila e avvia.
- Chiudere l'emulatore prima di ricompilare: su Windows il file ROM resta aperto durante l'esecuzione.

## Comandi

- **Frecce / joystick 1:** muovi il soldato.
- **Spazio / pulsante 1:** spara. Le casse possono essere distrutte; le guardie richiedono due colpi.
- **M:** mostra o chiude la pianta completa del piano; il riquadro rosso indica la posizione.
- **E:** nell'ascensore, in basso a sinistra nella pianta, passa al piano successivo.
- **1 / 2 / 3 / 4:** selezione diretta di sotterraneo / piano terra / primo / secondo piano, utile per esplorare e controllare le mappe. Si entra dall'ascensore.
- **ESC:** ricomincia.
- **V:** disattiva o riattiva la musica, lasciando disponibili gli effetti.

Si parte nella stanza originale al piano terra, con 99 colpi. La chiave e a sinistra del giocatore: raccoglierla permette di aprire la porta grigia a nord. La visuale segue il personaggio attraverso le stanze e i corridoi. Le porte aperte, gli oggetti raccolti e le casse distrutte sono conservati durante i cambi piano, fino a 128 modifiche per partita. Gli indicatori mostrano chiavi, munizioni, ferite, punteggio e piano; a 50 ferite la partita ricomincia.

## Colonna sonora originale

La ROM riproduce il tema melodico originale della versione C64: **50 battute in 3/4, 250 BPM, durata 36 secondi**, in ciclo continuo. Note, durate e tre parti vengono dalla trascrizione ABC di Stephan Pabst / Stephan64, ricavata dalle scritture al SID e pubblicata su C64-Wiki. Non e un nuovo brano composto per il prototipo. I timbri sono adattati ai chip MSX, quindi non e una registrazione o un'emulazione del SID C64.

`Gioca.cmd` avvia l'emulatore con l'espansione **Konami SCC**. La ROM la rileva automaticamente e usa cinque oscillatori wavetable: tre per le parti originali e due duplicazioni di timbro, alla stessa altezza, per dare corpo alla melodia e al basso. Il PSG resta disponibile per gli effetti. SCC e un'espansione compatibile con MSX1, non il chip audio presente di serie.

`Gioca PSG.cmd` avvia un MSX1 senza espansione. La stessa ROM riproduce le tre parti sul **PSG AY-3-8910** standard; per gli effetti condivide temporaneamente il canale del basso e poi ne ripristina la frequenza. Sono integrati sparo, raccolta, porta, colpo ricevuto e guardia eliminata.

Il player gira sull'interrupt BIOS H.TIMI, indipendentemente dal disegno delle stanze e della mappa; gestisce il ritmo a 50 tick al secondo e la compensazione per interrupt a 60 Hz. Il precedente hook BIOS viene conservato e concatenato. Nei test e stata verificata la macchina PAL C-BIOS_MSX1_EU in entrambe le configurazioni sonore.

`original-theme-scc.wav` e `original-theme-psg.wav` sono anteprime registrate dalla ROM nell'emulatore. La fonte musicale e archiviata in `assets/source/IntoEaglesNest_Theme.abc.txt`; `node scripts/import-original-music.cjs` controlla sincronizzazione delle voci, durata delle battute ed estensione delle note, e genera `src/original_music.h`.

## Risorse integrate

- **152 tile distinti** estratti dall'atlante C64, con casse, porte, tavoli, sedie, letti, piante e oggetti.
- **Quattro piani da 64x64 tile**, ciascuno di 512x512 pixel, con la disposizione delle stanze dell'atlante.
- **Otto fotogrammi direzionali per il protagonista e otto per le guardie**, derivati dalle tabelle della disassemblazione Amiga; i file sorgente contengono anche altri fotogrammi del protagonista.
- Compositore di sprite con controllo delle quattro presenze massime per scanline. I dettagli e le luci hanno priorita inferiore alle sagome e ai proiettili quando piu personaggi sono allineati.
- Pianta del piano consultabile nel gioco e anteprima PNG di tutti i piani in `assets/previews/four-floors-msx1.png`.

Provenienza, attribuzioni e formato di conversione sono documentati in **ASSET-SOURCES.md**. Le immagini degli atlanti non vengono dichiarate sorgenti originali del motore C64: sono il materiale effettivamente utilizzato per ricostruire i fondali e le piante. La logica dei singoli oggetti e una prima implementazione basata sulla loro classificazione grafica.

## Compila, verifica, rigenera

Da PowerShell nella cartella del progetto:

```powershell
./scripts/build.ps1
./scripts/run.ps1
./scripts/prepare-verification.ps1
```

La preparazione stampa il comando per eseguire la verifica nell'emulatore. La verifica usa l'emulatore per controllare movimento, chiave iniziale, porta, mappa, quattro piani, camera, munizioni, persistenza della porta e ascensore. Il resoconto della verifica eseguita durante lo sviluppo e salvato in `build/verification.txt`; le schermate vengono acquisite dalla ROM, non sono mockup. Gli spostamenti diretti di coordinate presenti nello script sono fixture di verifica, non percorsi di gioco simulati per intero.

Per modificare o riestrarre gli asset, con Node disponibile:

```powershell
node scripts/extract-source-assets.cjs
node scripts/import-original-assets.cjs
```

La seconda operazione verifica che compressione e decompressione delle quattro mappe restituiscano gli stessi tile; genera `src/original_assets.h`, il foglio tile, il foglio sprite e l'atlante dei piani convertiti. Per compilare normalmente non serve Node: l'header generato e gia incluso.

## Ambiente incluso

VS Code 1.143.1 portatile con estensione Microsoft C/C++ 1.34.4; Fusion-C 1.2 con sorgenti e documentazione; SDCC 4.6.0 #16555 dal bundle MSXgl; openMSX 21.0; C-BIOS MSX1 EU e logo dal repository openMSX. Tutti gli strumenti risiedono in questa cartella, senza modifiche al PATH globale o installazioni come amministratore.

SDCC viene usato con `--sdcccall 0`; le routine C Fusion-C utilizzate sono ricompilate con la stessa convenzione. `src/msx1_vram.c` evita la scrittura del registro MSX2 numero 14 sul TMS9918. La ROM e una cartuccia da 32768 byte con intestazione AB, codice a 0x4050 e RAM a 0xC000. Lo script di build verifica checksum Intel HEX, indirizzi e intestazione.

I lanciatori specificano `-romtype Normal`: la cartuccia del gioco e una ROM normale da 32 KB; la SCC e una seconda cartuccia sonora nell'emulatore. Questo evita che il riconoscimento automatico del mapper cambi con il contenuto del file.

Il pacchetto openMSX contiene due piccoli adattamenti ai percorsi Tcl per funzionare anche nell'ambiente Windows ristretto usato per preparare il progetto (`share/init.tcl`, `share/scripts/create_user_dirs.tcl`).

## Limiti attuali

Il rendering e MSX1 SCREEN 2, 256x192, con due colori per riga di otto pixel e palette fissa: alcuni colori e dettagli delle versioni C64/Amiga sono approssimati. La camera avanza per caratteri di otto pixel. Non e stato ancora provato su hardware reale.

Restano da completare: prigionieri e scorta, missioni, detonatori e fuga finale, logica esatta dei pass e dei singoli oggetti, IA e numero delle guardie, menu e bilanciamento. La presenza dei quattro piani non equivale al completamento del gioco originale.


## Prova online con WebMSX

https://eagle-nest-msx1-27ott-7k3r.grandbuddy11.chatgpt.site

Il sito ospita WebMSX C-BIOS 6.0.8 e questa ROM, con SCC in slot 2. Il link e privato del proprietario: accedere con il proprio account ChatGPT e premere Avvia il gioco. Non richiede un server locale. Sorgenti del sito in ../eagle-nest-web; identita e distribuzione registrate in .openai/hosting.json di quel progetto.
