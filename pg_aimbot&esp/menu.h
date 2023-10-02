#include "ESP.h"
#include "aimbot_esp.h"


//imgui stuff and other things left out

if(aimbot){
    ESP::DrawCircle((float)glWidth/2, (float)glHeight/2,fovCircleRadius, false, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
}

void draw_ESP(){
    if(!enemylistESP.empty()) {
        for (int i = 0; i < enemylistESP.size(); i++) {
            
            //drawing either red esp line if its enemy or green if its teammate 

            if (enemylistESP[i].enemy) {
                ESP::DrawLine(ImVec2((float) glWidth / 2, 0.0f),
                                ImVec2((float) glWidth - (glWidth - enemylistESP[i].screenPos.X),
                                        ((float) glHeight - enemylistESP[i].screenPos.Y - 10.0f)),
                                ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 3.0f);
                
            } else {
                ESP::DrawLine(ImVec2((float) glWidth / 2, 0.0f),
                                ImVec2((float) glWidth - (glWidth - enemylistESP[i].screenPos.X),
                                        ((float) glHeight - enemylistESP[i].screenPos.Y - 10.0f)),
                                ImVec4(0.0f, 1.0f, 0.0f, 1.0f), 3.0f);
                
            }
        }
        
    }

}
