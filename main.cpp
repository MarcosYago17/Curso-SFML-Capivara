#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <cmath> // Para a função 'ceil'

using namespace std;

// =======================================================
// ENUMERAÇÃO DE ESTADOS DO JOGO
// =======================================================
enum GameState {
    MENU,
    DIFFICULTY_CHOICE,
    PLAYING,
    GAME_OVER,
    OPTIONS_MENU, // Estado para a tela de opções
    TUTORIAL      // Retornando o estado para o tutorial
};

// =======================================================
// ESTRUTURAS E CONSTANTES DO JOGO
// =======================================================
const int NUM_HOLES = 9;
const float WINDOW_WIDTH = 1024.0f;
const float WINDOW_HEIGHT = 1024.0f;

// Definições de Dificuldade
struct DifficultySettings {
    float gameDuration;
    float minCapybaraDuration;
    float maxCapybaraDuration;
    int spawnRate;
};

DifficultySettings easy = {60.0f, 1.5f, 2.5f, 150};
DifficultySettings normal = {45.0f, 1.0f, 2.0f, 100};
DifficultySettings hard = {30.0f, 0.5f, 1.5f, 50};

DifficultySettings currentDifficulty;
int currentScore = 0;
sf::Clock gameClock;
sf::Time gameTimeLimit;

const float MOLE_OFFSET = 115.0f;
const float MOLE_RADIUS = 115.0f;

struct Hole {
    sf::Vector2f position;
    bool hasCapybara;
    sf::Clock capybaraTimer;
    float capybaraDuration;
};

vector<Hole> holes;

const sf::Vector2f HOLE_POSITIONS[NUM_HOLES] = {
    {56.0f + MOLE_OFFSET, 155.0f + MOLE_OFFSET},  // B1
    {413.0f + MOLE_OFFSET, 132.0f + MOLE_OFFSET}, // B2
    {746.0f + MOLE_OFFSET, 142.0f + MOLE_OFFSET}, // B3
    {230.0f + MOLE_OFFSET, 276.0f + MOLE_OFFSET}, // B4
    {620.0f + MOLE_OFFSET, 298.0f + MOLE_OFFSET}, // B5
    {415.0f + MOLE_OFFSET, 473.0f + MOLE_OFFSET}, // B6
    {71.0f + MOLE_OFFSET, 623.0f + MOLE_OFFSET},  // B7
    {388.0f + MOLE_OFFSET, 721.0f + MOLE_OFFSET}, // B8
    {720.0f + MOLE_OFFSET, 605.0f + MOLE_OFFSET}  // B9
};

// =======================================================
// FUNÇÕES AUXILIARES
// =======================================================

bool isCircleClicked(const sf::Vector2f& mousePos, const sf::Vector2f& center, float radius)
{
    float dx = mousePos.x - center.x;
    float dy = mousePos.y - center.y;
    float distanceSquared = dx * dx + dy * dy;
    return distanceSquared <= (radius * radius);
}

void initializeHoles() {
    holes.clear();
    for (int i = 0; i < NUM_HOLES; ++i) {
        Hole h;
        h.position = HOLE_POSITIONS[i];
        h.hasCapybara = false;
        h.capybaraDuration = 0.0f;
        holes.push_back(h);
    }
}

void startGame(const DifficultySettings& settings) {
    currentDifficulty = settings;
    currentScore = 0;
    gameTimeLimit = sf::seconds(settings.gameDuration);
    gameClock.restart();
    initializeHoles();
}

void spawnCapybara(Hole& hole) {
    hole.hasCapybara = true;
    hole.capybaraTimer.restart();
    float range = currentDifficulty.maxCapybaraDuration - currentDifficulty.minCapybaraDuration;
    hole.capybaraDuration = currentDifficulty.minCapybaraDuration + (float)rand() / (float)RAND_MAX * range;
}

void checkCapybaraClick(Hole& hole, const sf::Vector2f& mousePos) {
    if (hole.hasCapybara) {
        if (isCircleClicked(mousePos, hole.position, MOLE_RADIUS)) {
            hole.hasCapybara = false;
            currentScore++;
        }
    }
}

// =======================================================
// DEFINIÇÃO DAS FUNÇÕES DE TELA (Protótipos)
// =======================================================

void DrawMenu(sf::RenderWindow& window, const sf::Sprite& menuSprite);
void DrawDifficulty(sf::RenderWindow& window, const sf::Sprite& choiceSprite);
void DrawGame(sf::RenderWindow& window, const sf::Sprite& gameSprite, sf::Sprite& ToupeiraSprite, sf::Text& scoreText, sf::Text& timeText, sf::RectangleShape& timeBar);
void DrawGameOver(sf::RenderWindow& window, const sf::Sprite& gameBackgroundSprite, sf::Text& gameOverText, sf::Text& finalScoreText, sf::Text& clickToContinue);
void DrawOptions(sf::RenderWindow& window, const sf::Sprite& optionsMenuSprite,
                 sf::Text& muteXText, const sf::Vector2f& bgIconPos, const sf::Vector2f& clickIconPos,
                 bool isBackgroundSoundMuted, bool isClickSoundMuted);
void DrawTutorial(sf::RenderWindow& window, const sf::Sprite& tutorialMenuSprite, const sf::Font& font);

void HandleMenuEvents(sf::Event& event, sf::RenderWindow& window, GameState& currentState,
                      const sf::FloatRect& botaoPlay, const sf::FloatRect& botaoOptions,
                      const sf::FloatRect& botaoTutorial, const sf::FloatRect& botaoExit);

void HandleDifficultyEvents(sf::Event& event, sf::RenderWindow& window, GameState& currentState,
                            const sf::FloatRect& botaoEasy, const sf::FloatRect& botaoNormal,
                            const sf::FloatRect& botaoHard,
                            const sf::Vector2f& centerBack, float radiusBack);

void HandleGamingEvents(sf::Event& event, sf::RenderWindow& window, GameState& currentState);
void HandleOptionsEvents(sf::Event& event, sf::RenderWindow& window, GameState& currentState,
                         const sf::Vector2f& centerBackOptions, float radiusBackOptions,
                         const sf::FloatRect& botaoBackgroundSound, const sf::FloatRect& botaoClickSound,
                         bool& isBackgroundSoundMuted, bool& isClickSoundMuted, sf::Music& menuMusic);
void HandleTutorialEvents(sf::Event& event, sf::RenderWindow& window, GameState& currentState,
                          const sf::Vector2f& centerBackTutorial, float radiusBackTutorial);

// =======================================================
// FUNÇÃO PRINCIPAL (MAIN)
// =======================================================

int main()
{
    srand(time(NULL));
    GameState currentState = MENU;

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Capivara Whack-A-Mole (SFML)", sf::Style::Titlebar | sf::Style::Close);
    sf::View view(sf::FloatRect(0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT));
    window.setView(view);
    window.setFramerateLimit(60);

    // =======================================================
    // CARREGAMENTO DE RECURSOS
    // =======================================================

    sf::Texture menuInicialTexture;
    if (!menuInicialTexture.loadFromFile("inicial.png")) { // Esta é a sua imagem de 4 botões
       cout<<"Erro ao carregar textura inicial.png"<<endl; return -1;
    }
    sf::Sprite menuInicialSprite(menuInicialTexture);

    sf::Texture choiceBackgroundTexture;
    if (!choiceBackgroundTexture.loadFromFile("escolha.png")) {
       cout<<"Erro ao carregar a textura de escolha.png"<<endl; return -1;
    }
    sf::Sprite choiceBackgroundSprite(choiceBackgroundTexture);

    sf::Texture gameBackgroundTexture;
    if(!gameBackgroundTexture.loadFromFile("fundoGAME.png")){
             cout<<"Erro ao carregar a textura de fundoGAME.png"<<endl; return -1;
    }
    sf::Sprite gameBackgroundSprite(gameBackgroundTexture);

    sf::Texture ToupeiraTexture;
    if(!ToupeiraTexture.loadFromFile("toupeira.png")){
        cout<<"Erro ao carregar a toupeira.png"<<endl; return -1;
    }
    sf::Sprite ToupeiraSprite(ToupeiraTexture);
    ToupeiraSprite.setOrigin(ToupeiraTexture.getSize().x / 2.0f, ToupeiraTexture.getSize().y / 2.0f);

    sf::Texture optionsMenuTexture;
    if (!optionsMenuTexture.loadFromFile("options_menu.png")) {
        cout << "Erro ao carregar textura options_menu.png" << endl; return -1;
    }
    sf::Sprite optionsMenuSprite(optionsMenuTexture);

    sf::Texture tutorialMenuTexture;
    if (!tutorialMenuTexture.loadFromFile("escolha.png")) { // Reutilizando para tutorial por enquanto
        cout << "Erro ao carregar textura tutorial.png (usando escolha.png)" << endl; return -1;
    }
    sf::Sprite tutorialMenuSprite(tutorialMenuTexture);

    sf::Music menuInicialMusic;
    if(!menuInicialMusic.openFromFile("introSong.wav")){
        cout<<"Erro ao carregar o audio"<<endl;
    } else {
        menuInicialMusic.setLoop(true); menuInicialMusic.play();
    }

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        if (!font.loadFromFile("bin/Debug/arial.ttf")) {
            if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
                cerr << "ERRO FATAL: Nao foi possivel carregar a fonte 'arial.ttf'" << endl;
                return -1;
            }
        }
    }
    cout << "Fonte 'arial.ttf' carregada com sucesso!" << endl;

    // =======================================================
    // NOVO: Variáveis e Texto para o "X"
    // =======================================================
    bool isBackgroundSoundMuted = false;
    bool isClickSoundMuted = false;

    sf::Text muteXText;
    muteXText.setFont(font);
    muteXText.setString("X");
    muteXText.setCharacterSize(60);
    muteXText.setFillColor(sf::Color::Red);
    muteXText.setStyle(sf::Text::Bold);
    sf::FloatRect textBounds = muteXText.getLocalBounds();
    muteXText.setOrigin(textBounds.left + textBounds.width / 2.0f,
                        textBounds.top + textBounds.height / 2.0f);
    // =======================================================


    sf::Text scoreText("Pontos: 0", font, 40);
    scoreText.setFillColor(sf::Color::Black);
    scoreText.setPosition(50.0f, 50.0f);

    sf::Text timeText("Tempo: 60", font, 40);
    timeText.setFillColor(sf::Color::Black);
    timeText.setPosition(WINDOW_WIDTH - 250.0f, 50.0f);

    sf::RectangleShape timeBar(sf::Vector2f(WINDOW_WIDTH - 100.0f, 30.0f));
    timeBar.setFillColor(sf::Color::Green);
    timeBar.setPosition(50.0f, 10.0f);

    // =======================================================
    // COORDENADAS DOS BOTÕES (Do seu código)
    // =======================================================

    // Botões (Menu Principal - sua imagem mais recente)
    // X, Y, Largura, Altura
    sf::FloatRect botaoPlay(372, 532, 281, 75);      // PLAY
    sf::FloatRect botaoOptions(372, 630, 281, 75);   // OPTIONS
    sf::FloatRect botaoTutorial(372, 728, 281, 75);  // TUTORIAL
    sf::FloatRect botaoExit(372, 826, 281, 75);      // EXIT

    // Botões (Tela de Dificuldade - imagem escolha.png)
    sf::FloatRect botaoEasy(366, 488, 289, 67);
    sf::FloatRect botaoNormal(366, 620, 289, 67);
    sf::FloatRect botaoHard(366, 752, 289, 67);
    const sf::Vector2f centerBackDifficulty(122.0f, 883.0f);
    const float radiusBackDifficulty = 54.0f;

    // Botões (Tela de Options - imagem options_menu.png)
    sf::FloatRect botaoBackgroundSound(360, 570, 300, 85);
    sf::FloatRect botaoClickSound(360, 680, 300, 85);
    const sf::Vector2f centerBackOptions(122.0f, 883.0f); // Botão BACK
    const float radiusBackOptions = 54.0f;

    // =======================================================
    // ATUALIZADO: Coordenadas corrigidas para o "X"
    // =======================================================
    const sf::Vector2f backgroundSoundIconPos(650.0f, 625.0f); // Mais para a direita e para baixo
    const sf::Vector2f clickSoundIconPos(650.0f, 735.0f);      // Mais para a direita e para baixo


    // Botão Voltar da tela de TUTORIAL (usando as mesmas da dificuldade)
    const sf::Vector2f centerBackTutorial(122.0f, 883.0f);
    const float radiusBackTutorial = 54.0f;

    // Cursors
    sf::Cursor cursorHand;
    sf::Cursor cursorArrow;
    bool cursorIsHand = false;
    if (!cursorHand.loadFromSystem(sf::Cursor::Hand)) {}
    if (!cursorArrow.loadFromSystem(sf::Cursor::Arrow)) {}

    // Textos da tela de GAME OVER
    sf::Text gameOverText("FIM DE JOGO!", font, 60);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setPosition(WINDOW_WIDTH / 2.0f - gameOverText.getGlobalBounds().width / 2.0f, WINDOW_HEIGHT / 2.0f - 100.0f);

    sf::Text finalScoreText("", font, 40);
    finalScoreText.setFillColor(sf::Color::Black);

    sf::Text clickToContinue("Clique em qualquer lugar para voltar ao Menu.", font, 24);
    clickToContinue.setFillColor(sf::Color::Black);
    clickToContinue.setPosition(WINDOW_WIDTH / 2.0f - clickToContinue.getGlobalBounds().width / 2.0f, WINDOW_HEIGHT / 2.0f + 80.0f);


    // =======================================================
    // LOOP PRINCIPAL
    // =======================================================
    sf::Event event;

    while (window.isOpen())
    {
        // A. PROCESSAMENTO DE EVENTOS
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (currentState == MENU)
                {
                    HandleMenuEvents(event, window, currentState, botaoPlay, botaoOptions, botaoTutorial, botaoExit);
                }
                else if (currentState == DIFFICULTY_CHOICE)
                {
                    HandleDifficultyEvents(event, window, currentState, botaoEasy, botaoNormal, botaoHard, centerBackDifficulty, radiusBackDifficulty);
                }
                else if(currentState == PLAYING)
                {
                    HandleGamingEvents(event, window, currentState);
                }
                else if (currentState == GAME_OVER)
                {
                    currentState = MENU;
                    if (menuInicialMusic.getStatus() != sf::SoundSource::Playing) {
                        menuInicialMusic.play();
                    }
                }
                else if (currentState == OPTIONS_MENU)
                {
                    HandleOptionsEvents(event, window, currentState, centerBackOptions, radiusBackOptions,
                                        botaoBackgroundSound, botaoClickSound,
                                        isBackgroundSoundMuted, isClickSoundMuted, menuInicialMusic);
                }
                else if (currentState == TUTORIAL)
                {
                    HandleTutorialEvents(event, window, currentState, centerBackTutorial, radiusBackTutorial);
                }
            }
            if(event.type == sf::Event::KeyPressed){
                if(event.key.code == sf::Keyboard::Escape){
                    if(currentState == PLAYING || currentState == DIFFICULTY_CHOICE || currentState == OPTIONS_MENU || currentState == TUTORIAL){
                        currentState = MENU;
                        cout<<"Voltando ao menu principal."<<endl;
                        if (menuInicialMusic.getStatus() != sf::SoundSource::Playing) {
                            menuInicialMusic.play();
                        }
                    }
                }
            }
        }

        // B. LÓGICA DE ATUALIZAÇÃO DO JOGO
        if (currentState == PLAYING) {
            sf::Time elapsed = gameClock.getElapsedTime();
            sf::Time remainingTime = gameTimeLimit - elapsed;

            if (remainingTime.asSeconds() <= 0.0f) {
                currentState = GAME_OVER;
                finalScoreText.setString("Pontuação Final: " + to_string(currentScore));
                finalScoreText.setPosition(WINDOW_WIDTH / 2.0f - finalScoreText.getGlobalBounds().width / 2.0f, WINDOW_HEIGHT / 2.0f);
                if (menuInicialMusic.getStatus() == sf::SoundSource::Playing) {
                    menuInicialMusic.stop();
                }
            }

            float timeRatio = remainingTime.asSeconds() / currentDifficulty.gameDuration;
            timeBar.setSize(sf::Vector2f((WINDOW_WIDTH - 100.0f) * timeRatio, 30.0f));
            timeBar.setFillColor(timeRatio > 0.5f ? sf::Color::Green : (timeRatio > 0.2f ? sf::Color::Yellow : sf::Color::Red));

            ostringstream timeStream;
            timeStream << "Tempo: " << (int)ceil(remainingTime.asSeconds());
            timeText.setString(timeStream.str());

            ostringstream scoreStream;
            scoreStream << "Pontos: " << currentScore;
            scoreText.setString(scoreStream.str());

            for (int i = 0; i < NUM_HOLES; ++i) {
                if (!holes[i].hasCapybara) {
                    if (rand() % currentDifficulty.spawnRate == 0) {
                        spawnCapybara(holes[i]);
                    }
                } else {
                    if (holes[i].capybaraTimer.getElapsedTime().asSeconds() > holes[i].capybaraDuration) {
                        holes[i].hasCapybara = false;
                    }
                }
            }
        }

        // C. ATUALIZAÇÃO DO CURSOR (HOVER)
        sf::Vector2f worldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        bool isOverClickableArea = false;

        if (currentState == MENU) {
            isOverClickableArea = botaoPlay.contains(worldPos) ||
                                  botaoOptions.contains(worldPos) ||
                                  botaoTutorial.contains(worldPos) ||
                                  botaoExit.contains(worldPos);
        }
        else if (currentState == DIFFICULTY_CHOICE) {
             isOverClickableArea = botaoEasy.contains(worldPos) ||
                                     botaoNormal.contains(worldPos) ||
                                     botaoHard.contains(worldPos);
             if (!isOverClickableArea) {
                 isOverClickableArea = isCircleClicked(worldPos, centerBackDifficulty, radiusBackDifficulty);
             }
        }
        else if (currentState == PLAYING) {
            for (const auto& hole : holes) {
                if (hole.hasCapybara) {
                    if (isCircleClicked(worldPos, hole.position, MOLE_RADIUS)) {
                        isOverClickableArea = true;
                        break;
                    }
                }
            }
        }
        else if (currentState == OPTIONS_MENU) {
            isOverClickableArea = isCircleClicked(worldPos, centerBackOptions, radiusBackOptions) ||
                                  botaoBackgroundSound.contains(worldPos) ||
                                  botaoClickSound.contains(worldPos);
        }
        else if (currentState == TUTORIAL) {
             isOverClickableArea = isCircleClicked(worldPos, centerBackTutorial, radiusBackTutorial);
        }

        if (isOverClickableArea) {
            if (!cursorIsHand) { window.setMouseCursor(cursorHand); cursorIsHand = true; }
        }
        else {
            if (cursorIsHand) { window.setMouseCursor(cursorArrow); cursorIsHand = false; }
        }

        // D. DESENHO (RENDERIZAÇÃO)
        window.clear(sf::Color(100, 149, 237));

        if (currentState == MENU) {
                 DrawMenu(window, menuInicialSprite);
        }
        else if (currentState == DIFFICULTY_CHOICE) {
                 DrawDifficulty(window, choiceBackgroundSprite);
        }
        else if( currentState == PLAYING){
            DrawGame(window, gameBackgroundSprite, ToupeiraSprite, scoreText, timeText, timeBar);
        }
        else if (currentState == GAME_OVER) {
            DrawGameOver(window, gameBackgroundSprite, gameOverText, finalScoreText, clickToContinue);
        }
        else if (currentState == OPTIONS_MENU) {
            DrawOptions(window, optionsMenuSprite, muteXText,
                        backgroundSoundIconPos, clickSoundIconPos,
                        isBackgroundSoundMuted, isClickSoundMuted);
        }
        else if (currentState == TUTORIAL) {
            DrawTutorial(window, tutorialMenuSprite, font);
        }

        window.display();
    } // Fim do Loop Principal

    return 0;
} // Fim do main()

// =======================================================
// IMPLEMENTAÇÕES DE FUNÇÕES DE TELA
// =======================================================

void DrawMenu(sf::RenderWindow& window, const sf::Sprite& menuSprite)
{
    window.draw(menuSprite);
}

void DrawDifficulty(sf::RenderWindow& window, const sf::Sprite& choiceSprite)
{
    window.draw(choiceSprite);
}

void DrawGame( sf::RenderWindow& window, const sf::Sprite& gameSprite, sf::Sprite& ToupeiraSprite, sf::Text& scoreText, sf::Text& timeText, sf::RectangleShape& timeBar)
{
    window.draw(gameSprite);

    for (int i = 0; i < NUM_HOLES; ++i) {
        if (holes[i].hasCapybara) {
            ToupeiraSprite.setPosition(holes[i].position);
            window.draw(ToupeiraSprite);
        }
    }

    window.draw(scoreText);
    window.draw(timeText);
    window.draw(timeBar);
}

void DrawGameOver(sf::RenderWindow& window, const sf::Sprite& gameBackgroundSprite, sf::Text& gameOverText, sf::Text& finalScoreText, sf::Text& clickToContinue)
{
    window.draw(gameBackgroundSprite);
    window.draw(gameOverText);
    window.draw(finalScoreText);
    window.draw(clickToContinue);
}

void DrawOptions(sf::RenderWindow& window, const sf::Sprite& optionsMenuSprite,
                 sf::Text& muteXText, const sf::Vector2f& bgIconPos, const sf::Vector2f& clickIconPos,
                 bool isBackgroundSoundMuted, bool isClickSoundMuted)
{
    window.draw(optionsMenuSprite); // Desenha a imagem de fundo

    // Desenha o "X" se o som de fundo estiver mutado
    if (isBackgroundSoundMuted) {
        muteXText.setPosition(bgIconPos);
        window.draw(muteXText);
    }

    // Desenha o "X" se o som de clique estiver mutado
    if (isClickSoundMuted) {
        muteXText.setPosition(clickIconPos);
        window.draw(muteXText);
    }
}

void DrawTutorial(sf::RenderWindow& window, const sf::Sprite& tutorialMenuSprite, const sf::Font& font) {
    window.draw(tutorialMenuSprite); // Desenha o fundo

    sf::Text tutorialTitle("COMO JOGAR", font, 60);
    tutorialTitle.setFillColor(sf::Color::Black);
    tutorialTitle.setPosition(WINDOW_WIDTH / 2.0f - tutorialTitle.getGlobalBounds().width / 2.0f, 200.0f);
    window.draw(tutorialTitle);

    sf::Text tutorialText("Acerte as capivaras que aparecerem nos buracos.\n"
                          "Quanto mais rapido voce for, mais pontos.\n"
                          "Cuidado para nao deixar o tempo acabar!", font, 30);
    tutorialText.setFillColor(sf::Color::Black);
    tutorialText.setPosition(WINDOW_WIDTH / 2.0f - tutorialText.getGlobalBounds().width / 2.0f, 350.0f);
    window.draw(tutorialText);
}


void HandleMenuEvents(sf::Event& event, sf::RenderWindow& window, GameState& currentState,
                      const sf::FloatRect& botaoPlay, const sf::FloatRect& botaoOptions,
                      const sf::FloatRect& botaoTutorial, const sf::FloatRect& botaoExit)
{
    sf::Vector2f mousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    if (botaoPlay.contains(mousePosition)) {
        cout << "Botão Play Clicado - Transição para Dificuldade" << endl;
        currentState = DIFFICULTY_CHOICE;
    }
    else if (botaoOptions.contains(mousePosition)) {
        cout << "Botão Options Clicado - Transição para Opções" << endl;
        currentState = OPTIONS_MENU;
    }
    else if (botaoTutorial.contains(mousePosition)) {
        cout << "Botão Tutorial Clicado - Transição para Tutorial" << endl;
        currentState = TUTORIAL;
    }
    else if (botaoExit.contains(mousePosition)) {
        window.close();
    }
}

void HandleGamingEvents(sf::Event& event, sf::RenderWindow& window, GameState& currentState)
{
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
            for (int i = 0; i < NUM_HOLES; ++i) {
                checkCapybaraClick(holes[i], mousePos);
            }
        }
    }
}

void HandleDifficultyEvents(sf::Event& event, sf::RenderWindow& window, GameState& currentState,
                            const sf::FloatRect& botaoEasy, const sf::FloatRect& botaoNormal,
                            const sf::FloatRect& botaoHard,
                            const sf::Vector2f& centerBack, float radiusBack)
{
    sf::Vector2f mousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    if (isCircleClicked(mousePosition, centerBack, radiusBack))
    {
        currentState = MENU;
        cout << "Botão Voltar Clicado (Dificuldade)!" << endl;
    }
    else if (botaoEasy.contains(mousePosition)) {
        cout << "Dificuldade: FACIL selecionada! Indo para o jogo." << endl;
        startGame(easy);
        currentState = PLAYING;
    }
    else if (botaoNormal.contains(mousePosition)) {
        cout << "Dificuldade: NORMAL selecionada! Indo para o jogo." << endl;
        startGame(normal);
        currentState = PLAYING;
    }
    else if (botaoHard.contains(mousePosition)) {
        cout << "Dificuldade: DIFICIL selecionada! Indo para o jogo." << endl;
        startGame(hard);
        currentState = PLAYING;
    }
}

void HandleOptionsEvents(sf::Event& event, sf::RenderWindow& window, GameState& currentState,
                         const sf::Vector2f& centerBackOptions, float radiusBackOptions,
                         const sf::FloatRect& botaoBackgroundSound, const sf::FloatRect& botaoClickSound,
                         bool& isBackgroundSoundMuted, bool& isClickSoundMuted, sf::Music& menuMusic)
{
    sf::Vector2f mousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    if (isCircleClicked(mousePosition, centerBackOptions, radiusBackOptions))
    {
        currentState = MENU; // Volta para o menu principal
        cout << "Botão Voltar Clicado (Opções)!" << endl;
    }
    else if (botaoBackgroundSound.contains(mousePosition))
    {
        isBackgroundSoundMuted = !isBackgroundSoundMuted; // Inverte
        cout << "Botão Background Sound Clicado! Mudo: " << isBackgroundSoundMuted << endl;

        // Lógica para mutar a música
        if (isBackgroundSoundMuted) {
            menuMusic.setVolume(0);
        } else {
            menuMusic.setVolume(100);
        }
    }
    else if (botaoClickSound.contains(mousePosition))
    {
        isClickSoundMuted = !isClickSoundMuted; // Inverte
        cout << "Botão Click Sound Clicado! Mudo: " << isClickSoundMuted << endl;
        // Lógica para mutar os sons de clique (SFX)
    }
}

void HandleTutorialEvents(sf::Event& event, sf::RenderWindow& window, GameState& currentState,
                          const sf::Vector2f& centerBackTutorial, float radiusBackTutorial)
{
    sf::Vector2f mousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    if (isCircleClicked(mousePosition, centerBackTutorial, radiusBackTutorial))
    {
        currentState = MENU; // Volta para o menu principal
        cout << "Botão Voltar Clicado (Tutorial)!" << endl;
    }
}
