#include "PhysicsWorld2D.h"

#include "Log.h"
#include "CollisionShape.h"
#include <Box2D/Box2D.h>

//
// (c) 2018 Eduardo Doria.
//

using namespace Supernova;

class World2DContactListener: public b2ContactListener {
private:
    PhysicsWorld2D* world;

public:
    World2DContactListener(PhysicsWorld2D* world){
        this->world = world;
    }

    virtual void BeginContact(b2Contact* contact){
        CollisionShape* shapeA = (CollisionShape2D*)contact->GetFixtureA()->GetUserData();
        CollisionShape* shapeB = (CollisionShape2D*)contact->GetFixtureB()->GetUserData();

        world->call_onBeginContact(shapeA, shapeB);
    }

    virtual void EndContact(b2Contact* contact){
        CollisionShape* shapeA = (CollisionShape2D*)contact->GetFixtureA()->GetUserData();
        CollisionShape* shapeB = (CollisionShape2D*)contact->GetFixtureB()->GetUserData();

        world->call_onEndContact(shapeA, shapeB);
    }

};


PhysicsWorld2D::PhysicsWorld2D(): PhysicsWorld(){
    world = new b2World(b2Vec2(0.0f, 0.0f));

    contactListener = new World2DContactListener(this);
    world->SetContactListener(contactListener);

    velocityIterations = 8;
    positionIterations = 3;
}

PhysicsWorld2D::~PhysicsWorld2D(){
    for (int i = 0; i < bodies.size(); i++){
        ((Body2D*)bodies[i])->destroyBody();
    }
    delete contactListener;
    delete world;
}

b2World* PhysicsWorld2D::getBox2DWorld(){
    return world;
}

void PhysicsWorld2D::addBody(Body2D* body){
    bool founded = false;

    std::vector<Body*>::iterator it;
    for (it = bodies.begin(); it != bodies.end(); ++it) {
        if (body == (*it))
            founded = true;
    }

    if (!founded){
        bodies.push_back(body);

        body->createBody(this);
    }
}

void PhysicsWorld2D::removeBody(Body2D* body){
    std::vector<Body*>::iterator i = std::remove(bodies.begin(), bodies.end(), body);
    bodies.erase(i, bodies.end());

    body->destroyBody();
}

void PhysicsWorld2D::setGravity(Vector2 gravity){
    setGravity(gravity.x, gravity.y);
}

void PhysicsWorld2D::setGravity(float gravityX, float gravityY){
    world->SetGravity(b2Vec2(gravityX / S_POINTS_TO_METER_RATIO, gravityY / S_POINTS_TO_METER_RATIO));
}

void PhysicsWorld2D::setVelocityIterations(int velocityIterations){
    this->velocityIterations = velocityIterations;
}

void PhysicsWorld2D::setPositionIterations(int positionIterations){
    this->positionIterations = positionIterations;
}

Vector2 PhysicsWorld2D::getGravity(){
    b2Vec2 gravity = world->GetGravity();
    return Vector2(gravity.x * S_POINTS_TO_METER_RATIO, gravity.y * S_POINTS_TO_METER_RATIO);
}

int PhysicsWorld2D::getVelocityIterations(){
    return velocityIterations;
}

int PhysicsWorld2D::getPositionIterations(){
    return positionIterations;
}

void PhysicsWorld2D::step(float timeStep){
    world->Step(timeStep, velocityIterations, positionIterations);
}