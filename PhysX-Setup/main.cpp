// Ref: https://github.com/codetechandtutorials/PhysX-Setup
//
// This code demonstrates how to perform an overlap query in PhysX.
// The code creates a static cube and performs an overlap query with a box geometry.
// The code prints the names of the overlapping actors if any overlap is detected.
// The code can be used as a reference to perform overlap queries in PhysX.

#include <PxPhysicsAPI.h>
#include <iostream>

using namespace physx;

/// <summary>
/// Create a static cube actor
/// </summary>
/// <param name="physics"></param>
/// <param name="position"></param>
/// <param name="dimensions"></param>
/// <returns></returns>
physx::PxRigidStatic* createStaticCube(physx::PxPhysics* physics, const physx::PxVec3& position, const physx::PxVec3& dimensions) {
    physx::PxTransform transform(position);
    physx::PxShape* shape = physics->createShape(physx::PxBoxGeometry(dimensions), *physics->createMaterial(0.5f, 0.5f, 0.6f));
    physx::PxRigidStatic* actor = physics->createRigidStatic(transform);
    actor->attachShape(*shape);
    shape->release();
    return actor;
}

/// <summary>
/// Entrypoint of the program
/// </summary>
/// <returns></returns>
int main() {
    // Declare variables
    physx::PxDefaultAllocator mDefaultAllocatorCallback; // PhysX Allocator Callback
    physx::PxDefaultErrorCallback mDefaultErrorCallback; // PhysX Error Callback
    physx::PxDefaultCpuDispatcher* mDispatcher = NULL; // PhysX CPU Dispatcher
    physx::PxTolerancesScale mToleranceScale; // Tolerance scale for PhysX

    physx::PxFoundation* mFoundation = NULL; // PhysX Foundation
    physx::PxPhysics* mPhysics = NULL; // PhysX Physics
    physx::PxScene* mScene = NULL; // PhysX Scene
    physx::PxPvd* mPvd = NULL; // PhysX Visual Debugger

    // Init PhysX
    mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);
    if (!mFoundation) throw "PxCreateFoundation failed!";
    mPvd = PxCreatePvd(*mFoundation);

    // Connect to PhysX Visual Debugger
    physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    mPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);
    
    mToleranceScale.length = 1; // typical length of an object
    mToleranceScale.speed = 0; // nothing is moving
    
    // Create PhysX Physics
    mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, mToleranceScale, true, mPvd);

    // Create PhysX Scene Descriptor
    physx::PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
    sceneDesc.gravity = physx::PxVec3(0.0f, 0.0f, 0.0f); // no gravity
    
    // Create PhysX CPU Dispatcher
    mDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = mDispatcher;
    sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
    
    // Create PhysX Scene
    mScene = mPhysics->createScene(sceneDesc);
    
    // Create one static and one dynamic cube
    physx::PxRigidStatic* cube1 = createStaticCube(mPhysics, physx::PxVec3(0.1f, 0.2f, 0.3f), physx::PxVec3(1.0f, 1.0f, 1.0f));
    cube1->setName("Cube1");
    mScene->addActor(*cube1);
    
    // Create a box geometry for overlap query
    physx::PxBoxGeometry overlapBoxGeometry(physx::PxVec3(2.0f, 2.0f, 2.0f)); // Adjust dimensions as needed

    // Create a transform for the overlap region (position and orientation)
    physx::PxTransform overlapTransform(physx::PxVec3(0.0f, 0.0f, 0.0f)); // Set the position

    // Create a buffer to store the overlap results
    physx::PxOverlapHit hitBuffer[10]; // Adjust the buffer size as needed

    // Perform the overlap query
    physx::PxOverlapBuffer overlapResults(hitBuffer, 10); // Pass the buffer to the overlap query
    bool overlapStatus = mScene->overlap(overlapBoxGeometry, overlapTransform, overlapResults);

    if (overlapStatus) {
        // Handle the overlapping objects (e.g., print their names or perform other actions)
        for (PxU32 i = 0; i < overlapResults.nbTouches; ++i) {
            const PxOverlapHit& hit = overlapResults.touches[i];
            PxRigidActor* overlappingActor = hit.actor;
            if (overlappingActor)
            {
                std::cout << "Overlap detected with actor:"
                    << " name=" << overlappingActor->getName()
                    << " position=(" << overlappingActor->getGlobalPose().p.x
                    << "," << overlappingActor->getGlobalPose().p.y
                    << "," << overlappingActor->getGlobalPose().p.z
                    << ")" << std::endl;
            }
            else
            {
                std::cout << "Overlap detected with null actor" << std::endl;
            }
        }
    }
    else {
        std::cout << "No overlap detected." << std::endl;
    }

    // Cleanup: TODO

    return 0;
}
