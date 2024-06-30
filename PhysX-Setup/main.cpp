// Ref: https://github.com/codetechandtutorials/PhysX-Setup
//
// This code demonstrates how to perform an overlap query in PhysX.
// The code creates a static cube and performs an overlap query with a box geometry.
// The code prints the names of the overlapping actors if any overlap is detected.
// The code can be used as a reference to perform overlap queries in PhysX.

#include <PxPhysicsAPI.h>
#include <iostream>
#include <corecrt_math_defines.h>

using namespace physx;

/// <summary>
/// Create a static cube actor
/// </summary>
/// <param name="physics"></param>
/// <param name="position"></param>
/// <param name="dimensions"></param>
/// <returns></returns>
physx::PxRigidStatic* createStaticCube(physx::PxPhysics* physics, const physx::PxTransform& transform, const physx::PxVec3& dimensions) {
    physx::PxShape* shape = physics->createShape(physx::PxBoxGeometry(dimensions), *physics->createMaterial(0.5f, 0.5f, 0.6f));
    if (shape)
    {
        physx::PxRigidStatic* actor = physics->createRigidStatic(transform);
        if (actor)
        {
            actor->attachShape(*shape);
        }
        shape->release();
        return actor;
    }
    return nullptr;
}

/// <summary>
/// Function to convert Euler angles (in degrees) to a quaternion
/// </summary>
/// <param name="eulerAngles"></param>
/// <returns></returns>
PxQuat eulerAnglesToQuaternion(const float x, const float y, const float z) {
    // Convert Euler angles to radians if they are in degrees
    float pitch = x * M_PI / 180.0f;
    float yaw = y * M_PI / 180.0f;
    float roll = z * M_PI / 180.0f;

    // Normalize the angles to the range [-pi, pi]
    pitch = fmod(pitch + M_PI, 2 * M_PI) - M_PI;
    yaw = fmod(yaw + M_PI, 2 * M_PI) - M_PI;
    roll = fmod(roll + M_PI, 2 * M_PI) - M_PI;

    // Convert Euler angles to quaternions
    PxQuat qPitch = PxQuat(pitch, PxVec3(1.0f, 0.0f, 0.0f)); // Pitch around X-axis
    PxQuat qYaw = PxQuat(yaw, PxVec3(0.0f, 1.0f, 0.0f)); // Yaw around Y-axis
    PxQuat qRoll = PxQuat(roll, PxVec3(0.0f, 0.0f, 1.0f)); // Roll around Z-axis

    // Combine the quaternions
    PxQuat qCombined = qPitch * qYaw * qRoll;

    return qCombined;
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
    
    // Create one static cube
    physx::PxVec3 position1(0.1f, 0.2f, 0.3f);
    physx::PxQuat rotation1(eulerAnglesToQuaternion(45, 0, 0)); // degrees
    physx::PxTransform transform1(position1, rotation1);
    physx::PxRigidStatic* cube1 = createStaticCube(mPhysics, transform1, physx::PxVec3(1.0f, 1.0f, 1.0f));
    cube1->setName("Cube1");
    mScene->addActor(*cube1);
    
    // Create a box geometry for overlap query
    physx::PxBoxGeometry overlapBoxGeometry(physx::PxVec3(2.0f, 2.0f, 2.0f)); // Adjust dimensions as needed

    // Create a transform for the overlap region (position and orientation)
    physx::PxVec3 position2(0.5f, 0.6f, 0.7f);
    physx::PxQuat rotation2(eulerAnglesToQuaternion(-45, 0, 0)); // degrees
    physx::PxTransform transform2(position2, rotation2);

    // Create a buffer to store the overlap results
    physx::PxOverlapHit hitBuffer[10]; // Adjust the buffer size as needed

    // Perform the overlap query
    physx::PxOverlapBuffer overlapResults(hitBuffer, 10); // Pass the buffer to the overlap query
    bool overlapStatus = mScene->overlap(overlapBoxGeometry, transform2, overlapResults);

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
