#include "gamescene.h"
#include "engine.h"

#include <Box2D/Box2D.h>
#include <QDebug>


GameScene::GameScene() :
    m_world(new b2World(c_gravity)),
    m_pool(nullptr), m_diver(nullptr), m_water(nullptr),
    m_engine(new Engine(m_world))
{
//    setItemIndexMethod(QGraphicsScene::NoIndex);
    setSceneRect(QRectF(mapFromB2(c_world_top_left), mapFromB2(c_world_bottom_right)));
    connect(m_engine, SIGNAL(stepped()), this, SLOT(advance()), Qt::QueuedConnection);
}

GameScene::~GameScene()
{
    clear();
}

void GameScene::clear()
{
    m_engine->stopSimulation();
    m_engine->wait();
    QGraphicsScene::clear();
    delete m_world;
    delete m_engine;
    m_world = new b2World(c_gravity);
    m_engine = new Engine(m_world);
    connect(m_engine, SIGNAL(stepped()), this, SLOT(advance()), Qt::QueuedConnection);
    m_pool = nullptr;
    m_diver = nullptr;
    m_water = nullptr;
}

void GameScene::createPool()
{
    Q_ASSERT(!m_pool);
    b2BodyDef def;
    def.position = {0, 0};
    m_pool = new Pool(m_world->CreateBody(&def));
    addItem(m_pool);
    advance();
}

void GameScene::createDiver(const b2Vec2 &pos)
{
    Q_ASSERT(!m_diver); // diver should only be created once
    b2BodyDef def;
    def.position = pos;
    QVector<b2Body*> part_bodies;   // a diver has 6 body parts
    for (int i = 0; i < 4; ++i) {
        part_bodies << m_world->CreateBody(&def);  // create an empty b2body
    }
    // and then pass it to Diver() to construct a Diver with concrete shapes and physical attributes
    m_diver = new Diver(part_bodies);
    addItem(m_diver);   // to scene
    advance();
}

void GameScene::createWater(const b2Shape &shape, const b2Vec2 &pos)
{
    Q_ASSERT(!m_water);
    b2ParticleSystemDef def;
    m_water = new Water(m_world->CreateParticleSystem(&def), shape, pos);
    addItem(m_water);
    advance();
}

void GameScene::initialize(size_t step)
{
    Q_ASSERT(!m_engine->isBusy());
    for (size_t i = 0; i < step; ++i)
        m_world->Step(c_time_step, c_velocity_iterations, c_position_iterations, c_particle_iterations);
}

Actor *GameScene::createDebugBall(const b2Vec2 &pos)
{
    bool flag = m_engine->isBusy();
    if (flag) {
        m_engine->stopSimulation();
        m_engine->wait(1000);
    }
    b2BodyDef def;
    def.position = pos;
    def.type = b2_dynamicBody;
    auto body = m_world->CreateBody(&def);
    auto ball = new Actor(body);
    b2CircleShape shape;
    shape.m_radius = 0.2;
    body->CreateFixture(&shape, 2.0);
    if (flag)
        m_engine->asyncSimulate();
    addItem(ball);
    advance();
    return ball;
}

void GameScene::destroyActor(Actor *actor)
{
    bool flag = m_engine->isBusy();
    if (flag) {
        m_engine->stopSimulation();
        m_engine->wait(1000);
    }
    removeItem(actor);
    m_world->DestroyBody(actor->m_body);
    delete actor;
    advance();
    if (flag)
        m_engine->asyncSimulate();
}

Diver &GameScene::diver()
{
    Q_ASSERT(m_diver);
    return *m_diver;
}

Engine &GameScene::engine()
{
    Q_ASSERT(m_engine);
    return *m_engine;
}

void GameScene::stopSimulation()
{
    m_engine->stopSimulation();
}

void GameScene::asyncSimulate()
{
    m_engine->asyncSimulate();
}

void GameScene::syncSimulate()
{
    m_engine->syncSimulate();
}
