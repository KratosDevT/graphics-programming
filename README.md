# graphics-programming
Arkanoid-clone project with Home-Made Game Engine in Directx11/12


## Architettura Generale
Il progetto segue il pattern MVC (Model-View-Controller) tipico dei motori grafici.

main.cpp -> avvia SystemClass

### SystemClass - Il Controller Principale

Gestisce il loop principale del gioco, input e timing

- Calcola il deltaTime per animazioni fluide
- Gestisce i controlli (frecce per muovere, +/- per ridimensionare)
- Coordina ApplicationClass per il rendering

### ApplicationClass - La Logica di Gioco
È il "cervello" del gioco che:

- Inizializza tutti gli oggetti grafici
- Gestisce la logica di gioco (collisioni, vittoria/sconfitta)
- Coordina il rendering di tutti gli elementi

### Classi di Rendering
#### D3DClass - Wrapper DirectX 11
- Swap chain (doppio buffering)
- Device e DeviceContext (interfacce DirectX)
- Render target (dove disegnare)
- Depth buffer (profondità 3D, anche se usi 2D)
- Viewport (area di rendering)

#### CameraClass - Sistema di Coordinate
- View matrix

#### TextureShaderClass - Shader per Texture
- Vertex Shader: disegna i vertici
- Pixel Shader: colora i pixel a partire dal colore del vertice
- Gestisce le texture (immagini sui poligoni)

###  Classi degli Oggetti di Gioco

#### BitmapClass - La Paletta del Giocatore
- Posizione + velocità + raggio
- Rimbalzo sui bordi schermo
- Collisioni con rettangoli (paletta e mattoni)
- UpdateWithCollision(): fisica completa

#### BrickClass - I Mattoni da Distruggere
- Flag m_visible per nascondere mattoni colpiti
- Ottimizzazione: non renderizza se invisibile

#### TextureClass - Caricamento Immagini

- Converte formato TGA → texture DirectX
- Gestisce memoria GPU per le immagini


### Come Funziona il Rendering

#### Vertex Buffer Pattern
Ogni oggetto crea 6 vertici (2 triangoli = 1 quadrato): \
// Esempio coordinate per un quadrato: \
vertices[0] = {left, top, 0.0f};      // Top left \
vertices[1] = {right, bottom, 0.0f};  // Bottom right \
vertices[2] = {left, bottom, 0.0f};   // Bottom left \
// Secondo triangolo...

#### Coordinate Transform
// Da coordinate schermo a DirectX: \
left = (screenWidth / 2 * -1) + objectX; \
top = (screenHeight / 2) - objectY; \


#### Loop di Rendering
ApplicationClass::Render(): \
1. Clear screen (nero) \
2. Setup matrici (world, view, projection) \
3. Disabilita Z-buffer (rendering 2D) \
4. Renderizza paletta \
5. Renderizza mattoni visibili \
6. Renderizza palla \
7. Riabilita Z-buffer \
8. Present (mostra frame) \


### Logica di Gioco


#### Collisioni Cerchio-Rettangolo
1. Trova punto più vicino del rettangolo al centro cerchio
2. Calcola distanza
3. Se distanza < raggio → collisione
4. Determina lato colpito per rimbalzo corretto

#### Game State
- m_remainingBricks: conta mattoni rimasti
- IsGameWon(): vinto se remainingBricks = 0  
- IsGameOver(): perso se palla esce dal fondo
- ResetGame(): ripristina tutto

### Ottimizzazioni
#### Buffer Update Optimization
Gli oggetti aggiornano i vertex buffer solo se la posizione cambia
#### Visibility Culling: 
I mattoni distrutti non vengono renderizzati
#### Delta Time
Movimento indipendente dal framerate
#### Dynamic Buffers
Solo per oggetti che si muovono