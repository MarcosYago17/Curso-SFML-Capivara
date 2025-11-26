#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <cmath>

using namespace std;

// =======================================================
// ENUMERAÇÃO DE ESTADOS DO JOGO
// =======================================================
enum GameState {
    MENU,
    DIFFICULTY_CHOICE,
    PLAYING,
    GAME_OVER,
    OPTIONS_MENU,
    TUTORIAL
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
    string name;
};

DifficultySettings easy = {60.0f, 1.5f, 2.5f, 150, "FACIL"};
DifficultySettings normal = {45.0f, 1.0f, 2.0f, 100, "NORMAL"};
DifficultySettings hard = {30.0f, 0.5f, 1.5f, 50, "DIFICIL"};

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
    cout << "Jogo iniciado! Dificuldade: " << settings.name << endl;
}

void spawnCapybara(Hole& hole) {
    hole.hasCapybara = true;
    hole.capybaraTimer.restart();
    float range = currentDifficulty.maxCapybaraDuration - currentDifficulty.minCapybaraDuration;
    hole.capybaraDuration = currentDifficulty.minCapybaraDuration + (float)rand() / (float)RAND_MAX * range;
}

void checkCapybaraClick(Hole& hole, const sf::Vector2f& mousePos, sf::Sound& clickSound, bool isClickSoundMuted) {
    if (hole.hasCapybara) {
        if (isCircleClicked(mousePos, hole.position, MOLE_RADIUS)) {
            if(!isClickSoundMuted) {
                clickSound.play();
            }
            hole.hasCapybara = false;
            currentScore++;
            cout << "ACERTOU! Pontos: " << currentScore << endl;
        }
    }
}

// =======================================================
// DEFINIÇÃO DAS FUNÇÕES DE TELA (Protótipos)
// =======================================================

void DrawMenu(sf::RenderWindow& window, const sf::Sprite& menuSprite);

void DrawDifficulty(sf::RenderWindow& window, const sf::Sprite& choiceSprite);

void DrawGame(sf::RenderWindow& window, const sf::Sprite& gameSprite, sf::Sprite& ToupeiraSprite,
              sf::Text& scoreText, sf::Text& timeText, sf::RectangleShape& timeBar);

void DrawGameOver(sf::RenderWindow& window, const sf::Sprite& gameBackgroundSprite,
                  sf::Text& gameOverText, sf::Text& finalScoreText, sf::Text& difficultyText,
                  sf::Text& clickToContinue, sf::RectangleShape& scorePanel);

void DrawOptions(sf::RenderWindow& window, const sf::Sprite& optionsMenuSprite,
                 sf::Text& muteXText, const sf::Vector2f& bgIconPos, const sf::Vector2f& clickIconPos,
                 bool isBackgroundSoundMuted, bool isClickSoundMuted);

void DrawTutorial(sf::RenderWindow& window, const sf::Sprite& tutorialMenuSprite, const sf::Font& font);

void checkCapybaraClick(Hole& hole, const sf::Vector2f& mousePos, sf::Sound& clickSound, bool isClickSoundMuted);

void HandleMenuEvents(sf::Event& event, sf::RenderWindow& window, GameState& currentState,
                      const sf::FloatRect& botaoPlay, const sf::FloatRect& botaoOptions,
                      const sf::FloatRect& botaoTutorial, const sf::FloatRect& botaoExit, sf::Sound& clickSound, bool isClickSoundMuted);

void HandleDifficultyEvents(sf::Event& event, sf::RenderWindow& window, GameState& currentState,
                            const sf::FloatRect& botaoEasy, const sf::FloatRect& botaoNormal,
                            const sf::FloatRect& botaoHard,
                            const sf::Vector2f& centerBack, float radiusBack, sf::Sound& clickSound, bool isClickSoundMuted);

void HandleGamingEvents(sf::Event& event, sf::RenderWindow& window, GameState& currentState, sf::Sound& clickSound, bool isClickSoundMuted);

void HandleOptionsEvents(sf::Event& event, sf::RenderWindow& window, GameState& currentState,
                         const sf::Vector2f& centerBackOptions, float radiusBackOptions,
                         const sf::FloatRect& botaoBackgroundSound, const sf::FloatRect& botaoClickSound,
                         bool& isBackgroundSoundMuted, bool& isClickSoundMuted, sf::Music& menuMusic, sf::Sound& clickSound);

void HandleTutorialEvents(sf::Event& event, sf::RenderWindow& window, GameState& currentState,
                          const sf::Vector2f& centerBackTutorial, float radiusBackTutorial, sf::Sound& clickSound, bool isClickSoundMuted);

// =======================================================
// FUNÇÃO PRINCIPAL (MAIN)
// =======================================================

int main()
{
    srand(static_cast<unsigned>(time(NULL)));
    GameState currentState = MENU;

    sf::RenderWindow window(sf::VideoMode(static_cast<unsigned>(WINDOW_WIDTH), static_cast<unsigned>(WINDOW_HEIGHT)),
                           "Capivara Whack-A-Mole (SFML)", sf::Style::Titlebar | sf::Style::Close);
    sf::View view(sf::FloatRect(0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT));
    window.setView(view);
    window.setFramerateLimit(60);

    // =======================================================
    // CARREGAMENTO DE RECURSOS
    // =======================================================

    sf::Texture menuInicialTexture;
    if (!menuInicialTexture.loadFromFile("inicial.png")) {
       cout << "Erro ao carregar textura inicial.png" << endl; return -1;
    }
    sf::Sprite menuInicialSprite(menuInicialTexture);

    sf::Texture choiceBackgroundTexture;
    if (!choiceBackgroundTexture.loadFromFile("escolha.png")) {
       cout << "Erro ao carregar a textura de escolha.png" << endl; return -1;
    }
    sf::Sprite choiceBackgroundSprite(choiceBackgroundTexture);

    sf::Texture gameBackgroundTexture;
    if(!gameBackgroundTexture.loadFromFile("fundoGAME.png")){
        cout << "Erro ao carregar a textura de fundoGAME.png" << endl; return -1;
    }
    sf::Sprite gameBackgroundSprite(gameBackgroundTexture);

    sf::Texture ToupeiraTexture;
    if(!ToupeiraTexture.loadFromFile("toupeira.png")){
        cout << "Erro ao carregar a toupeira.png" << endl; return -1;
    }
    sf::Sprite ToupeiraSprite(ToupeiraTexture);
    ToupeiraSprite.setOrigin(ToupeiraTexture.getSize().x / 2.0f, ToupeiraTexture.getSize().y / 2.0f);

    sf::Texture optionsMenuTexture;
    if (!optionsMenuTexture.loadFromFile("options_menu.png")) {
        cout << "Erro ao carregar textura options_menu.png" << endl; return -1;
    }
    sf::Sprite optionsMenuSprite(optionsMenuTexture);

    sf::Texture tutorialMenuTexture;
    if (!tutorialMenuTexture.loadFromFile("escolha.png")) {
        cout << "Aviso: usando escolha.png para tutorial" << endl;
    }
    sf::Sprite tutorialMenuSprite(tutorialMenuTexture);

    sf::Music menuInicialMusic;
    if(!menuInicialMusic.openFromFile("introSong.wav")){
        cout << "Erro ao carregar o audio introSong.wav" << endl;
    } else {
        menuInicialMusic.setLoop(true);
        menuInicialMusic.play();
    }

    sf::SoundBuffer clickSoundBuffer;
    if(!clickSoundBuffer.loadFromFile("click.wav")){
        cout<< "Erro ao carregar o audio de clique" << endl;
        return -1;
    }
    sf::Sound clickSound;
    clickSound.setBuffer(clickSoundBuffer);

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
    // VARIÁVEIS DE ÁUDIO E CONTROLES
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
    // TEXTOS DO JOGO
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
    // TEXTOS DA TELA DE GAME OVER (MELHORADA)
    // =======================================================
    sf::Text gameOverText("FIM DE JOGO!", font, 70);
    gameOverText.setFillColor(sf::Color(220, 20, 60)); // Vermelho intenso
    gameOverText.setStyle(sf::Text::Bold);

    sf::Text finalScoreText("", font, 50);
    finalScoreText.setFillColor(sf::Color::White);
    finalScoreText.setStyle(sf::Text::Bold);

    sf::Text difficultyText("", font, 35);
    difficultyText.setFillColor(sf::Color(255, 215, 0)); // Dourado

    sf::Text clickToContinue("Clique em qualquer lugar para voltar ao Menu", font, 28);
    clickToContinue.setFillColor(sf::Color::White);

    // Painel de pontuação
    sf::RectangleShape scorePanel(sf::Vector2f(600.0f, 400.0f));
    scorePanel.setFillColor(sf::Color(0, 0, 0, 180)); // Preto semi-transparente
    scorePanel.setOutlineThickness(5.0f);
    scorePanel.setOutlineColor(sf::Color::White);

    // =======================================================
    // COORDENADAS DOS BOTÕES
    // =======================================================

    // Botões do Menu Principal
    sf::FloatRect botaoPlay(372, 532, 281, 75);      // PLAY
    sf::FloatRect botaoOptions(372, 630, 281, 75);   // OPTIONS
    sf::FloatRect botaoTutorial(372, 728, 281, 75);  // TUTORIAL
    sf::FloatRect botaoExit(372, 870, 281, 75);      // EXIT

    // Botões da Tela de Dificuldade
    sf::FloatRect botaoEasy(366, 488, 289, 67);
    sf::FloatRect botaoNormal(366, 620, 289, 67);
    sf::FloatRect botaoHard(366, 752, 289, 67);
    const sf::Vector2f centerBackDifficulty(122.0f, 883.0f);
    const float radiusBackDifficulty = 54.0f;

    // Botões da Tela de Options
    sf::FloatRect botaoBackgroundSound(360, 570, 300, 85);
    sf::FloatRect botaoClickSound(360, 680, 300, 85);
    const sf::Vector2f centerBackOptions(122.0f, 883.0f);
    const float radiusBackOptions = 54.0f;

    const sf::Vector2f backgroundSoundIconPos(650.0f, 625.0f);
    const sf::Vector2f clickSoundIconPos(650.0f, 735.0f);

    // Botão Voltar do Tutorial
    const sf::Vector2f centerBackTutorial(122.0f, 883.0f);
    const float radiusBackTutorial = 54.0f;

    // Cursores
    sf::Cursor cursorHand;
    sf::Cursor cursorArrow;
    bool cursorIsHand = false;
    if (!cursorHand.loadFromSystem(sf::Cursor::Hand)) {}
    if (!cursorArrow.loadFromSystem(sf::Cursor::Arrow)) {}

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
                    HandleMenuEvents(event, window, currentState, botaoPlay, botaoOptions, botaoTutorial, botaoExit, clickSound, isClickSoundMuted);
                }
                else if (currentState == DIFFICULTY_CHOICE)
                {
                    HandleDifficultyEvents(event, window, currentState, botaoEasy, botaoNormal, botaoHard,
                                         centerBackDifficulty, radiusBackDifficulty, clickSound, isClickSoundMuted);
                }
                else if(currentState == PLAYING)
                {
                    HandleGamingEvents(event, window, currentState, clickSound, isClickSoundMuted);
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
                                      isBackgroundSoundMuted, isClickSoundMuted, menuInicialMusic, clickSound);
                }
                else if (currentState == TUTORIAL)
                {
                    HandleTutorialEvents(event, window, currentState, centerBackTutorial, radiusBackTutorial, clickSound, isClickSoundMuted);
                }
            }

            if(event.type == sf::Event::KeyPressed){
                if(event.key.code == sf::Keyboard::Escape){
                    if(currentState == PLAYING || currentState == DIFFICULTY_CHOICE ||
                       currentState == OPTIONS_MENU || currentState == TUTORIAL){
                        currentState = MENU;
                        cout << "Voltando ao menu principal." << endl;
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

                // Atualiza textos do Game Over
                finalScoreText.setString("PONTUACAO: " + to_string(currentScore));
                difficultyText.setString("Dificuldade: " + currentDifficulty.name);

                if (menuInicialMusic.getStatus() == sf::SoundSource::Playing) {
                    menuInicialMusic.stop();
                }

                cout << "Fim de Jogo! Pontuacao: " << currentScore << " (Dificuldade: "
                     << currentDifficulty.name << ")" << endl;
            }

            float timeRatio = remainingTime.asSeconds() / currentDifficulty.gameDuration;
            timeBar.setSize(sf::Vector2f((WINDOW_WIDTH - 100.0f) * timeRatio, 30.0f));
            timeBar.setFillColor(timeRatio > 0.5f ? sf::Color::Green :
                                (timeRatio > 0.2f ? sf::Color::Yellow : sf::Color::Red));

            ostringstream timeStream;
            timeStream << "Tempo: " << static_cast<int>(ceil(remainingTime.asSeconds()));
            timeText.setString(timeStream.str());

            ostringstream scoreStream;
            scoreStream << "Pontos: " << currentScore;
            scoreText.setString(scoreStream.str());

            // Lógica das Capivaras (Sistema de Spawn)
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
        else if(currentState == PLAYING){
            DrawGame(window, gameBackgroundSprite, ToupeiraSprite, scoreText, timeText, timeBar);
        }
        else if (currentState == GAME_OVER) {
            DrawGameOver(window, gameBackgroundSprite, gameOverText, finalScoreText,
                        difficultyText, clickToContinue, scorePanel);
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
    }

    return 0;
}

// =======================================================
// IMPLEMENTAÇÕES DAS FUNÇÕES DE TELA
// =======================================================

void DrawMenu(sf::RenderWindow& window, const sf::Sprite& menuSprite)
{
    window.draw(menuSprite);
}

void DrawDifficulty(sf::RenderWindow& window, const sf::Sprite& choiceSprite)
{
    window.draw(choiceSprite);
}

void DrawGame(sf::RenderWindow& window, const sf::Sprite& gameSprite, sf::Sprite& ToupeiraSprite,
              sf::Text& scoreText, sf::Text& timeText, sf::RectangleShape& timeBar)
{
    window.draw(gameSprite);

    // Desenha todas as capivaras visíveis
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

void DrawGameOver(sf::RenderWindow& window, const sf::Sprite& gameBackgroundSprite,
                  sf::Text& gameOverText, sf::Text& finalScoreText, sf::Text& difficultyText,
                  sf::Text& clickToContinue, sf::RectangleShape& scorePanel)
{
    window.draw(gameBackgroundSprite);

    // Centraliza o painel
    scorePanel.setPosition(WINDOW_WIDTH / 2.0f - scorePanel.getSize().x / 2.0f,
                          WINDOW_HEIGHT / 2.0f - scorePanel.getSize().y / 2.0f);
    window.draw(scorePanel);

    // Centraliza os textos
    sf::FloatRect bounds = gameOverText.getGlobalBounds();
    gameOverText.setPosition(WINDOW_WIDTH / 2.0f - bounds.width / 2.0f,
                            WINDOW_HEIGHT / 2.0f - 150.0f);
    window.draw(gameOverText);

    bounds = finalScoreText.getGlobalBounds();
    finalScoreText.setPosition(WINDOW_WIDTH / 2.0f - bounds.width / 2.0f,
                              WINDOW_HEIGHT / 2.0f - 50.0f);
    window.draw(finalScoreText);

    bounds = difficultyText.getGlobalBounds();
    difficultyText.setPosition(WINDOW_WIDTH / 2.0f - bounds.width / 2.0f,
                              WINDOW_HEIGHT / 2.0f + 30.0f);
    window.draw(difficultyText);

    bounds = clickToContinue.getGlobalBounds();
    clickToContinue.setPosition(WINDOW_WIDTH / 2.0f - bounds.width / 2.0f,
                               WINDOW_HEIGHT / 2.0f + 120.0f);
    window.draw(clickToContinue);
}

void DrawOptions(sf::RenderWindow& window, const sf::Sprite& optionsMenuSprite,
                 sf::Text& muteXText, const sf::Vector2f& bgIconPos, const sf::Vector2f& clickIconPos,
                 bool isBackgroundSoundMuted, bool isClickSoundMuted)
{
    window.draw(optionsMenuSprite);

    if (isBackgroundSoundMuted) {
        muteXText.setPosition(bgIconPos);
        window.draw(muteXText);
    }

    if (isClickSoundMuted) {
        muteXText.setPosition(clickIconPos);
        window.draw(muteXText);
    }
}

void DrawTutorial(sf::RenderWindow& window, const sf::Sprite& tutorialMenuSprite, const sf::Font& font)
{
    window.draw(tutorialMenuSprite);

    // Título
    sf::Text tutorialTitle("COMO JOGAR", font, 70);
    tutorialTitle.setFillColor(sf::Color::Black);
    tutorialTitle.setStyle(sf::Text::Bold);
    sf::FloatRect bounds = tutorialTitle.getGlobalBounds();
    tutorialTitle.setPosition(WINDOW_WIDTH / 2.0f - bounds.width / 2.0f, 150.0f);
    window.draw(tutorialTitle);

    // Painel de fundo para o texto
    sf::RectangleShape textPanel(sf::Vector2f(800.0f, 450.0f));
    textPanel.setFillColor(sf::Color(255, 255, 255, 220));
    textPanel.setOutlineThickness(4.0f);
    textPanel.setOutlineColor(sf::Color::Black);
    textPanel.setPosition(WINDOW_WIDTH / 2.0f - 400.0f, 280.0f);
    window.draw(textPanel);

    // Instruções detalhadas
    sf::Text instruction1("1. Clique nas CAPIVARAS que aparecem nos buracos", font, 32);
    instruction1.setFillColor(sf::Color::Black);
    instruction1.setPosition(150.0f, 320.0f);
    window.draw(instruction1);

    sf::Text instruction2("2. Cada capivara acertada vale 1 ponto", font, 32);
    instruction2.setFillColor(sf::Color::Black);
    instruction2.setPosition(150.0f, 380.0f);
    window.draw(instruction2);

    sf::Text instruction3("3. As capivaras fogem rapidamente!", font, 32);
    instruction3.setFillColor(sf::Color::Black);
    instruction3.setPosition(150.0f, 440.0f);
    window.draw(instruction3);

    sf::Text instruction4("4. Fique atento ao tempo restante", font, 32);
    instruction4.setFillColor(sf::Color::Black);
    instruction4.setPosition(150.0f, 500.0f);
    window.draw(instruction4);

    sf::Text instruction5("5. Escolha a dificuldade que preferir:", font, 32);
    instruction5.setFillColor(sf::Color::Black);
    instruction5.setPosition(150.0f, 560.0f);
    window.draw(instruction5);

    sf::Text instruction6("   - FACIL: 60s | Capivaras lentas", font, 28);
    instruction6.setFillColor(sf::Color(0, 128, 0));
    instruction6.setPosition(150.0f, 605.0f);
    window.draw(instruction6);

    sf::Text instruction7("   - NORMAL: 45s | Velocidade media", font, 28);
    instruction7.setFillColor(sf::Color(255, 140, 0));
    instruction7.setPosition(150.0f, 645.0f);
    window.draw(instruction7);

    sf::Text instruction8("   - DIFICIL: 30s | Capivaras rapidas!", font, 28);
    instruction8.setFillColor(sf::Color::Red);
    instruction8.setPosition(150.0f, 685.0f);
    window.draw(instruction8);

    // Dica
    sf::Text tip("DICA: Pressione ESC para voltar ao menu a qualquer momento", font, 24);
    tip.setFillColor(sf::Color(50, 50, 50));
    tip.setStyle(sf::Text::Italic);
    bounds = tip.getGlobalBounds();
    tip.setPosition(WINDOW_WIDTH / 2.0f - bounds.width / 2.0f, 780.0f);
    window.draw(tip);
}

// =======================================================
// IMPLEMENTAÇÕES DAS FUNÇÕES DE EVENTOS
// =======================================================

void HandleMenuEvents(sf::Event& event, sf::RenderWindow& window, GameState& currentState,
                      const sf::FloatRect& botaoPlay, const sf::FloatRect& botaoOptions,
                      const sf::FloatRect& botaoTutorial, const sf::FloatRect& botaoExit, sf::Sound& clickSound, bool isClickSoundMuted)
{
    sf::Vector2f mousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    if (botaoPlay.contains(mousePosition)) {
        if(!isClickSoundMuted) {
            clickSound.play();
        }
        cout << "Botao Play Clicado - Transicao para Dificuldade" << endl;
        currentState = DIFFICULTY_CHOICE;
    }
    else if (botaoOptions.contains(mousePosition)) {
        if(!isClickSoundMuted) {
            clickSound.play();
        }
        cout << "Botao Options Clicado - Transicao para Opcoes" << endl;
        currentState = OPTIONS_MENU;
    }
    else if (botaoTutorial.contains(mousePosition)) {
        if(!isClickSoundMuted) {
            clickSound.play();
        }
        cout << "Botao Tutorial Clicado - Transicao para Tutorial" << endl;
        currentState = TUTORIAL;
    }
    else if (botaoExit.contains(mousePosition)) {
        if(!isClickSoundMuted) {
            clickSound.play();
        }
        cout << "Botao Exit Clicado - Fechando jogo" << endl;
        window.close();
    }
}

void HandleGamingEvents(sf::Event& event, sf::RenderWindow& window, GameState& currentState, sf::Sound& clickSound, bool isClickSoundMuted)
{
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
            for (int i = 0; i < NUM_HOLES; ++i) {
                checkCapybaraClick(holes[i], mousePos, clickSound, isClickSoundMuted);
            }
        }
    }
}

void HandleDifficultyEvents(sf::Event& event, sf::RenderWindow& window, GameState& currentState,
                            const sf::FloatRect& botaoEasy, const sf::FloatRect& botaoNormal,
                            const sf::FloatRect& botaoHard,
                            const sf::Vector2f& centerBack, float radiusBack, sf::Sound& clickSound, bool isClickSoundMuted)
{
    sf::Vector2f mousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    if (isCircleClicked(mousePosition, centerBack, radiusBack))
    {
        if(!isClickSoundMuted) {
            clickSound.play();
        }
        currentState = MENU;
        cout << "Botao Voltar Clicado (Dificuldade)!" << endl;
    }
    else if (botaoEasy.contains(mousePosition)) {
        if(!isClickSoundMuted) {
            clickSound.play();
        }
        startGame(easy);
        currentState = PLAYING;
    }
    else if (botaoNormal.contains(mousePosition)) {
        if(!isClickSoundMuted) {
            clickSound.play();
        }
        startGame(normal);
        currentState = PLAYING;
    }
    else if (botaoHard.contains(mousePosition)) {
        if(!isClickSoundMuted) {
            clickSound.play();
        }
        startGame(hard);
        currentState = PLAYING;
    }
}

void HandleOptionsEvents(sf::Event& event, sf::RenderWindow& window, GameState& currentState,
                         const sf::Vector2f& centerBackOptions, float radiusBackOptions,
                         const sf::FloatRect& botaoBackgroundSound, const sf::FloatRect& botaoClickSound,
                         bool& isBackgroundSoundMuted, bool& isClickSoundMuted, sf::Music& menuMusic, sf::Sound& clickSound)
{
    sf::Vector2f mousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    if (isCircleClicked(mousePosition, centerBackOptions, radiusBackOptions))
    {
        if(!isClickSoundMuted) {
            clickSound.play();
        }
        currentState = MENU;
        cout << "Botao Voltar Clicado (Opcoes)!" << endl;
    }
    else if (botaoBackgroundSound.contains(mousePosition))
    {
        if(!isClickSoundMuted) {
            clickSound.play();
        }
        isBackgroundSoundMuted = !isBackgroundSoundMuted;
        cout << "Bot ao Background Sound Clicado! Mudo: " << isBackgroundSoundMuted << endl;

        if (isBackgroundSoundMuted) {
            menuMusic.setVolume(0);
        } else {
            menuMusic.setVolume(100);
        }
    }
    else if (botaoClickSound.contains(mousePosition))
    {
        if(!isClickSoundMuted) {
            clickSound.play();
        }
        isClickSoundMuted = !isClickSoundMuted;
        cout << "Botao Click Sound Clicado! Mudo: " << isClickSoundMuted << endl;
    }
}

void HandleTutorialEvents(sf::Event& event, sf::RenderWindow& window, GameState& currentState,
                          const sf::Vector2f& centerBackTutorial, float radiusBackTutorial, sf::Sound& clickSound, bool isClickSoundMuted)
{
    sf::Vector2f mousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    if (isCircleClicked(mousePosition, centerBackTutorial, radiusBackTutorial))
    {
        if(!isClickSoundMuted) {
            clickSound.play();
        }
        currentState = MENU;
        cout << "Botao Voltar Clicado (Tutorial)!" << endl;
    }
}
