#include <SFML/Graphics.hpp>

using namespace sf;


// Const values
static const float defaultSwitchTime = 0.1f;
static const Color bg(90, 90, 90);


// Uninstantiatable!
// p____
class Prop        
{
public:
    void DrawTo(RenderWindow& window)
    {
        sprite.setPosition(pos.x, pos.y);
        window.draw(sprite);
    }

public:
    Vector2i size;
    Vector2f pos;
    Sprite sprite;
};

// Still prop
class StaticProp : public Prop  // (sprite, width, height, x, y)    
{
public:
    StaticProp(Sprite pSprite, int pWidth, int pHeight, float pX, float pY)
    {
        sprite = pSprite;

        size.x = pWidth;
        size.y = pHeight;
        pos.x = pX;
        pos.y = pY;
    }

    void SetSprite(Sprite pSprite)
    {
        sprite = pSprite;
    }
};



// ====== Animation ======


// Contains all of the sprites, indexed.
// ss____
struct Spritesheet      
{
public:
    Spritesheet(Texture pTexture, uint pRows, uint pColumns)
    : texture(pTexture), rows(pRows), columns(pColumns)
    {
        width = pTexture.getSize().x;
        height = pTexture.getSize().y;

        sprite.setTexture(texture);

        // Loads individual sprites into the frame vector
        for(int y = 0; y < pRows; y++)
        {
            for (int x = 0; x < pColumns; x++)
            {
                sprite.setTextureRect(IntRect(
                    x * (width / columns),          // Starting X
                    y * (height / rows),         // Starting Y
                    (width / columns),
                    (height / rows)
                ));

                frame.push_back(sprite);
            }
        }
    }

public:
    std::vector<Sprite> frame;          // Holds all of the sprite frames from the spritesheet

private:
    Texture texture;
    uint width, height, rows, columns;
    Sprite sprite;
};

// Contains a vector of all of the animation sprites.
//a____
class Anim  // (spritesheet)
{
public:
    Anim(Spritesheet pSpritesheet)
    : spritesheet(pSpritesheet), switchTime(defaultSwitchTime)
    {}

    // Adds a frame to the animation
    void addFrame(uint index)
    {
        frame.push_back( spritesheet.frame[index] );
    }

    // Plays the animation
    Sprite play(float dt)
    {
        animTime += dt;

        if(animTime >= switchTime)
        {
            animTime -= switchTime;
            currentFrame++;

            if(currentFrame >= frame.size())
            {
                currentFrame = 0;
            }
        }

        return frame[currentFrame];
    }


private:
    Spritesheet spritesheet;
    std::vector<Sprite> frame;

    int currentFrame = 0;
    float animTime = 0.0f;
    float switchTime = 0.0f;
};


// Animated Prop
class AnimProp : public Prop    // (width, height, x, y, default anim)
{
public:
    AnimProp(int pWidth, int pHeight, float pX, float pY, Anim pAnim)
    : anim(pAnim)
    {
        size.x = pWidth;
        size.y = pHeight;
        pos.x = pX;
        pos.y = pY;
    }

    void update(float dt)
    {
        sprite = anim.play(dt);
    }

    void setAnim(Anim pAnim)
    {
        anim = pAnim;
    }

public:
    Anim anim;
};




class Human : public AnimProp 
{
public:

    Human(int pWidth, int pHeight, float pX, float pY, Anim pAnim, Anim paRunL, Anim paRunR, Anim paStandL, Anim paStandR)
    : AnimProp(pWidth, pHeight, pX, pY, pAnim), aRunL(paRunL), aRunR(paRunR), aStandL(paStandL), aStandR(paStandR)
    {}

    enum PlayerState
    {
        RunningLeft,
        RunningRight,
        StandingLeft,
        StandingRight
    };


    void runLeft(float dt)
    {
        pos.x -= runningSpeed * dt;
        setPlayerState(RunningLeft);
        facingLeft = true;
    }

    void runRight(float dt)
    {
        pos.x += runningSpeed * dt;
        setPlayerState(RunningRight);
        facingLeft = false;
    }

    void stopRunning()
    {
        if(facingLeft)
            setPlayerState(StandingLeft);
        else
            setPlayerState(StandingRight);
    }


private:

    void setPlayerState(PlayerState pPlayerState)
    {
        if(playerState != pPlayerState)
        {
            playerState = pPlayerState;

            switch(playerState)
            {
                case RunningLeft:
                    anim = aRunL;
                    break;
                
                case RunningRight:
                    anim = aRunR;
                    break;

                case StandingLeft:
                    anim = aStandL;
                    break;

                case StandingRight:
                    anim = aStandR;
                    break;
            }
        }
    }

private:

    PlayerState playerState = StandingRight;

    Anim aRunL; // run left
    Anim aRunR; // run right
    Anim aStandL;
    Anim aStandR;

    bool facingLeft = false;

    int runningSpeed = 500;
};



//
// MAIN
//



int main()
{
    // Window setup
    const int WIN_WIDTH = 1200;
    const int WIN_HEIGHT = 600;
    RenderWindow window(VideoMode(WIN_WIDTH, WIN_HEIGHT), "SFML works!");
    // View Setup
    View vMain(FloatRect(0.0, 0.0, WIN_WIDTH, WIN_HEIGHT));
    window.setView(vMain);



    //
    // Player
    //

    Texture tPlayer; tPlayer.loadFromFile("images/maincharacter.png");
    Spritesheet ssPlayer(tPlayer, 2, 4);

    // Player standing animations
    Anim aPlayerStandL(ssPlayer);
    aPlayerStandL.addFrame(0);

    Anim aPlayerStandR(ssPlayer);
    aPlayerStandR.addFrame(4);

    // Player running animations
    Anim aPlayerRunL(ssPlayer);
    aPlayerRunL.addFrame(1);
    aPlayerRunL.addFrame(2);
    aPlayerRunL.addFrame(3);

    Anim aPlayerRunR(ssPlayer);
    aPlayerRunR.addFrame(5);
    aPlayerRunR.addFrame(6);
    aPlayerRunR.addFrame(7);

    Human player(10, 10, 50.0f, 50.0f, aPlayerStandR, 
    aPlayerRunL, aPlayerRunR, aPlayerStandL, aPlayerStandR);
    



    // Pre-game setup
    float dt = 0.0f;
    Clock clock;

    while (window.isOpen())
    {
        dt = clock.restart().asSeconds();


        //
        // INPUT
        //

        Event event;
        while (window.pollEvent(event))
        {
            switch(event.type)
            {
                case Event::Closed:
                    window.close();
                    break;
            }
        }

        if(Keyboard::isKeyPressed(Keyboard::A))
            player.runLeft(dt);
        else if(Keyboard::isKeyPressed(Keyboard::D))
            player.runRight(dt);
        else
            player.stopRunning();

        //
        // END INPUT
        //


        // Update stuff
        player.update(dt);


        window.clear(bg);

        // Draw stuff
        player.DrawTo(window);

        window.display();
    }

    return 0;
}

