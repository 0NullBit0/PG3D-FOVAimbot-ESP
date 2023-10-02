#include <cmath>
#include <iostream>
#include "Quaternion.h"
#include "Unity.h"
#include "Vector3.h"
/**/
//Software designed by nullBit 
/**/
// pixel gun 3d aimbot and esp on version 23.5.1
//hooks done with dobby

bool aimbot, esp;

struct enemy_t{
    Vector3 screenPos;
    bool enemy;
    // you can add more info for the esp like health, distance etc.. 
};

std::vector<enemy_t> enemylistESP;
std::vector<void*> playerlist;
void* localPlayer;
void* myCam;
float fovCircleRadius = 10.0f;

class PlayerlistManager{
    public:
        PlayerlistManager();
    
    void tryAdd(void* obj){
        playerlist.push_back(obj);
    }

    void tryRemove(void* obj){
        for(int i = 0; i<playerlist.size(); i++){
            if(playerlist[i] == obj)
                playerlist.erase(playerlist.begin()+i);
        }
    }

    ~PlayerlistManager(){}
};

PlayerlistManager* playerlistManager;

bool isMine(void* player){
    if(player){
        void* skinName = *(void**)((uint64_t)player + 0x660);
        return *(bool*)((uint64_t) skinName + 0xC0);
    }
    return false;
}

void* get_playerDamageable(void* player){
    if(player){
        void* damageable = *(void**)((uint64_t)player + 0x638);
        return damageable;
    }
    return nullptr;
}

void* get_playerTransform(void* player){
    if(player){
        void* transform = *(void**)((uint64_t)player + 0x398);
        return transform;
    }
    return nullptr;
}

void* get_baseCameraTransform(void* player){
    void* skinName = *(void**)((uint64_t)player + 0x660);
    if(skinName != nullptr){
        void* firstPersonControlSharp = *(void**)((uint64_t)skinName + 0x1E0);
        if(firstPersonControlSharp != nullptr){
            void* baseCamera = *(void**)((uint64_t)firstPersonControlSharp + 0x1A0);
            if(baseCamera != nullptr){
                return *(void**)((uint64_t)baseCamera + 0x48);
            }
        }
    }
}

void* (*get_cam)();
bool (*isEnemy)(void* myDamageable, void* player);
bool (*isDead)(void* damageable);
void* (*Component$get_transform)(void* component);
Vector3 (*get_position)(void* transform);
Vector3 (*worldToScreen)(void* camera, Vector3 worldPosition);
Vector3 (*Transform$inverseTransformPoint)(void* transform,Vector3 position);
void (*Transform$rotate)(void* transform,float x, float y, float z);
Quaternion (*Transform$get_rotation)(void* transform);

void initFnPointers(){
    get_cam = (void*(*)())(il2cpp.start + 0x46A3414);
    isEnemy = (bool(*)(void*, void*))(il2cpp.start + 0x46EC664);
    isDead = (bool(*)(void*))(il2cpp.start + 0x46EC9DC);
    Component$get_transform = (void*(*)(void*))(il2cpp.start + 0x46BE01C);
    get_position = (Vector3(*)(void*))(il2cpp.start + 0x46C9B5C);
    worldToScreen = (Vector3(*)(void*, Vector3))(il2cpp.start + 0x46A31E0);
    Transform$get_rotation = (Quaternion(*)(void*))(il2cpp.start + 0x46C9DF4);
    Transform$rotate = (void(*)(void*,float, float, float))(il2cpp.start + 0x46CAC10);
    Transform$inverseTransformPoint = (Vector3(*)(void*,Vector3))(il2cpp.start + 0x46CB3B8);
}

void(*origPlayerMoveC$update)(void* obj);
void PlayerMoveC$update(void* obj){
    if(obj != nullptr){
        if(aimbot){
            if(isMine(obj))
                localPlayer = obj;
            
            myCam = get_cam();

            if(playerlist.size() > 1 && localPlayer != nullptr && myCam != nullptr){
                for(int i = 1; i < playerlist.size(); i++){ // loop starting at 1 cuz 0 is not a player
                    void* enemy = playerlist[i];
                    void* myDamageable = get_playerDamageable(localPlayer);
                    void* enemyDamageable = get_playerDamageable(enemy);
                    if(isEnemy(myDamageable, enemy) && !isDead(enemyDamageable)){
                        void* myTransform = Component$get_transform(get_playerTransform(localPlayer));
                        void* enemyTransform = Component$get_transform(get_playerTransform(enemy));
                        
                        Vector3 myPosition = get_position(myTransform);
                        Vector3 enemyPosition = get_position(enemyTransform);

                        Vector3 enemyScreenPosition = worldToScreen(cam, enemyPosition);
                        Vector3 direction = enemyPosition - myPosition;

                        void* baseCamTransform = get_baseCameraTransform(localPlayer);
                        Quaternion myRotation = Transform$get_rotation(baseCamTransform);

                        Vector3 targetEnemyVector = Transform$inverseTransformPoint(baseCamTransform, enemyPosition);
                        // calculating pitch and yaw angles in degrees 
                        float deltaYaw = (std::atan2(targetEnemyVector.Z, targetEnemyVector.X)) * 180/M_PI;
                        float distanceVector = 
                            std::sqrt(targetEnemyVector.X*targetEnemyVector.X + targetEnemyVector.Z*targetEnemyVector.Z);
                        
                        float pitchOffset = 0.65f; // this is applied to aim on head body etc you can implement a switch on this
                        // 0.65 as offset will aim on head
                        float deltaPitch = (std::atan2(targetEnemyVector.Y + 0.65f, distanceVector)) *180/M_PI;

                        float screenX = (float )glWidth-((float)glWidth- enemyScreenPosition.X);
                        float screenY = ((float)glHeight - enemyScreenPosition.Y);
                        float screenDistance2Middle = sqrt(pow((screenX-(glWidth/2)),2) + pow((screenY-(glHeight/2)),2));

                        if(screenDistance2Middle <= fovCircleRadius && enemyScreenPosition.Z >= 1){
                            /*here we rotate basecamtrans for pitch and our trans for yaw
                            to prevent any faulty z axis rotation so the aimbot will look clean :)
                            we also 
                            invert the rotation (-) since positive rot means rotating downwards(pitch) or right(yaw) */ 
                            Transform$rotate(baseCamTransform, -(deltaPitch), 0.f, 0.f);
                            Transform$rotate(myTransform, 0.f, -(deltaYaw-90.0f), 0.f); // -90 so 0 degrees corresponds to z axis(forward!) do NOT change
                        }
                    }

                }
            }
        }

        if(esp){
            if(isMine(obj))
                localPlayer = obj;
            myCam = get_cam();

            if(myCam != nullptr && !playerlist.empty() && localPlayer != nullptr){
                
                enemylistESP.resize(0);
                for(int i = 1; i < playerlist.size(); i++){
                    void* enemy = playerlist[i];
                    Vector3 enemyPosition = get_position(Component$get_transform(get_playerTransform(enemy)));
                    Vector3 enemyScreenPosition = worldToScreen(myCam, enemyPosition);
                    if(glHeight - enemyScreenPosition >= glHeight)
                        continue;
                    enemy_t enemyPush;
                    if(enemyScreenPosition.Z >= 1 && !isDead(get_playerDamageable(enemy))){
                        enemyPush.screenPos = enemyScreenPosition;
                        if(isEnemy(get_playerDamageable(localPlayer), enemy))
                            enemyPush.enemy = true;
                        else
                            enemyPush.enemy = false;
                        enemylistESP.push_back(enemyPush);
                    }
                }
                    
                
            }
            if(playerlist.empty())
                enemylistESP.resize(0);
        }

    }
    origPlayerMoveC$update(obj);
}


void(*origPlayerConstructor)(void* obj);
void playerConstructor(void* obj){
    if(obj != nullptr){
        playerlistManager->tryAdd(obj);
    }
    origPlayerConstructor(obj);
}

void(*origPlayerDestroy)(void* obj);
void playerDestroy(void* obj){
    if(obj != nullptr){
        playerlistManager->tryRemove(obj);
    }
    origPlayerDestroy(obj);
}

void doHooks(){
    DobbyHook((void*)il2cpp.start+0x458E3D8, (void*)playerConstructor, (void**)origPlayerConstructor);
    DobbyHook((void*)il2cpp.start+0x45C0DD0, (void*)playerDestroy, (void**)origPlayerDestroy);
    DobbyHook((void*)il2cpp.start+0x45C6464, (void*)PlayerMoveC$update, (void**)origPlayerMoveC$update);
}

