#include <iostream>
#include <Windows.h>
#include "Off.h"
#include "world.h"
#include "Vector3.h"
#include <fstream>
#include "includes.h"
#include "math.h"
#include "dx.h"
#include "includes.h"
#include <string>



uintptr_t moduleBase;

struct BoneMatrix_t {
	byte pad1[12];
	float x;
	byte pad2[12];
	float y;
	byte pad3[12];
	float z;
};



Vector3* aimRecoilPunch;

template<typename T> T RPM(uintptr_t address) {
	try { return *(T*)address; }
	catch (...) { return T(); }
}


uintptr_t getLocalPlayer() {
	return RPM< uintptr_t>(moduleBase + dwLocalPlayer);
}


uintptr_t getPlayer(int index) {
	return RPM< uintptr_t>(moduleBase + dwEntityList + index * 0x10); // use index times 0x10 because the distance between each player 0x10.
}

int getTeam(uintptr_t player) {
	return RPM<int>(player + m_iTeamNum);
}

int GetPlayerHealth(uintptr_t player) {
    return RPM<int>(player + m_iHealth);
}

int GetLocalPlayerHealth() {
	return RPM<int>(getLocalPlayer() + m_iHealth);
}

int getCrosshairID(uintptr_t player) {
	return RPM<int>(player + m_iCrosshairId);
}

Vector3 PlayerLocation(uintptr_t player) { //Stores XYZ coordinates in a Vector3.
	return RPM<Vector3>(player + m_vecOrigin);
}

int getLocalFlags()
{
	return RPM<int>(getLocalPlayer() + m_fFlags);
}

int screenX = GetSystemMetrics(SM_CXSCREEN);
int screenY = GetSystemMetrics(SM_CYSCREEN);

struct Vector4 {
	float x, y, z, a;
};


/*

bool WorldToScreen(Vector3 pos, Vector2& screen) {
	Vector4 clipCoords;

	clipCoords.x = pos.x * matrix[0] + pos.y * matrix[1] + pos.z * matrix[2] + matrix[3];
	clipCoords.y = pos.x * matrix[4] + pos.y * matrix[5] + pos.z * matrix[6] + matrix[7];
	clipCoords.z = pos.x * matrix[8] + pos.y * matrix[9] + pos.z * matrix[10] + matrix[11];
	clipCoords.a = pos.x * matrix[12] + pos.y * matrix[13] + pos.z * matrix[14] + matrix[15];

	if (clipCoords.a < 0.1f) {
		return false;
	}
	Vector3 NDC;
	NDC.x = clipCoords.x / clipCoords.a;
	NDC.y = clipCoords.y / clipCoords.a;
	NDC.z = clipCoords.z / clipCoords.a;

	screen.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
	screen.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);

	

	return true;
}


*/

HDC hdc;

#define TORAD(x) ((x) * 0.1745329252)


float matrix[16];

void Update() {
	memcpy(&matrix, (BYTE*)(moduleBase + dwViewMatrix), sizeof(matrix));
}

bool WorldToScreen(Vector3 pos, Vector2& screen)
{
	screen.x = matrix[0] * pos.x + matrix[1] * pos.y + matrix[2] * pos.z + matrix[3];
	screen.y = matrix[4] * pos.x + matrix[5] * pos.y + matrix[6] * pos.z + matrix[7];

	const float w = matrix[12] * pos.x + matrix[13] * pos.y + matrix[14] * pos.z + matrix[15];

	if (w < 0.1f)
		return false;

	screen.x /= w;
	screen.y /= w;



	screen.x = windowWidth * 0.5f/*.97f*/ + screen.x * windowWidth * 0.5f/*.97f*/;
	screen.y = windowHeight * 0.5f/*1.047f*/ - screen.y * windowHeight * 0.5f/*1.047f*/;//1.f original

	//screen.x = 0.5f * (1.f + screen.x / w);
	//screen.y = 0.5f * (1.f - screen.y / w);

	//screen.x = screen.x * screen.x;
	//screen.y = screen.y * screen.y;

	return true;
}

void* d3d9Device[119];
BYTE EndSceneBytes[7]{ 0 };
tEndScene oEndScene = nullptr;
extern LPDIRECT3DDEVICE9 pDevice = nullptr;
Hack* hack;


WNDPROC wndproc_original = NULL;



WNDPROC oWndProc;



extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//error

//extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

LRESULT WINAPI WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	//if (true)
	//	return ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
		return true;
	}
		
	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
	//return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool toggleGlowEsp = true;
bool ShowMenu = true;
bool toggleBhop = true;
bool toggleTriggerBot = true;
bool toggleSnapLines = true;
bool toggleBoxESP = true;
bool EditTopBarColorandMenuColor = false;

void APIENTRY hkEndScene(LPDIRECT3DDEVICE9 o_pDevice) {
	if (!pDevice)
		pDevice = o_pDevice;

	DrawFilledRect(25, 25, 100, 100, D3DCOLOR_ARGB(255, 255, 255, 255));

	

	static bool init = true;
	if (init) {

		D3DDEVICE_CREATION_PARAMETERS CP;
		pDevice->GetCreationParameters(&CP);
		window = CP.hFocusWindow;
		init = false;
		
		ImGui::CreateContext();
		
		ImGuiIO& io = ImGui::GetIO();
		//io.ConfigWindowsMoveFromTitleBarOnly == true;
		//io.WantCaptureMouse == true;
		
		//ImGui_ImplWin32_Init(GetProcessWindow());//error
		io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
		ImGui_ImplWin32_Init(window);//window
		ImGui_ImplDX9_Init(pDevice);
		oWndProc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)WndProc);//window
		
	}
	
	
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();//error
	ImGui::NewFrame();

	ImGui::Begin("GamaCS");
	if (ImGui::BeginTabBar("sad")) {
		if (ImGui::BeginTabItem("ESP")) {
			//ImGui::Checkbox("TriggerBot", &toggleTriggerBot);
			ImGui::Checkbox("GlowESP", &toggleGlowEsp);
			ImGui::Checkbox("BoxESP", &toggleBoxESP);
			ImGui::Checkbox("SnapLines", &toggleSnapLines);
			
			
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("MISC")) {
			ImGui::Checkbox("BHOP", &toggleBhop);
			ImGui::Checkbox("TriggerBot", &toggleTriggerBot);
			ImGui::EndTabItem();
		}
	}
	//ImGui::Text("Hello, world!");
	
	
	//ImGui::Checkbox("EditTopBarColor&MenuColor", &EditTopBarColorandMenuColor);
	
	
	
	
	
	//static float menuColor[4] = { 1.0f,1.0f,1.0f,1.0f };
	//static float topColor[4] = { 0.4f, 0.7f, 0.0f, 0.5f };


	

	//ImGui::ColorEdit3("topColor", topColor);
	//ImGui::ColorEdit3("menuColor", menuColor);

	//ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImColor(topColor[0], topColor[1], topColor[2], topColor[3]);


		ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImColor(0, 20, 0, 255);
		ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImColor(0,200,0,255);
		ImGui::GetStyle().Colors[ImGuiCol_Tab] = ImColor(0, 100, 0, 255);
		ImGui::GetStyle().Colors[ImGuiCol_TabActive] = ImColor(0, 190, 0, 255);
		ImGui::GetStyle().Colors[ImGuiCol_TabHovered] = ImColor(0, 65, 0, 255);
		ImGui::GetStyle().Colors[ImGuiCol_TitleBgCollapsed] = ImColor(0, 200, 0, 255);
		ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = ImColor(0, 200, 0, 255);
		ImGui::GetStyle().Colors[ImGuiCol_ResizeGrip] = ImColor(0, 255, 0, 255);
		ImGui::GetStyle().Colors[ImGuiCol_ResizeGripHovered] = ImColor(0, 100, 0, 255);
		ImGui::GetStyle().Colors[ImGuiCol_ResizeGripActive] = ImColor(0, 50, 0, 255);
		ImGui::GetStyle().Colors[ImGuiCol_Border] = ImColor(0, 255, 0, 255);
		ImGui::GetStyle().Colors[ImGuiCol_BorderShadow] = ImColor(0, 255, 0, 255);
		ImGui::GetStyle().Colors[ImGuiCol_CheckMark] = ImColor(0, 255, 0, 255);
		ImGui::GetStyle().Colors[ImGuiCol_FrameBg] = ImColor(0, 100, 0, 255);
		ImGui::GetStyle().Colors[ImGuiCol_Header] = ImColor(255, 0, 0, 255);	
		ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered] = ImColor(0, 50, 0, 255); 
		ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive] = ImColor(0, 50, 0, 255); 
		ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered] = ImColor(0, 100, 0, 255);
	

	//menuColor[4] = 10.0f, 1.0f, 1.0f, 1.0f;

	//ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImColor(255, 101, 53, 255);
	//ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImColor(255, 101, 53, 255);

	ImGui::End();
	ImGui::Render();
	ImGui::EndFrame();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	
	
	for (int i = 1; i <= 32; i++) {


		uintptr_t localplayer = *(uintptr_t*)(moduleBase + dwLocalPlayer);
		if (localplayer == NULL) {
			continue;
		}

		if (localplayer != NULL) {


			int localplayerTeam = *(int*)(localplayer + m_iTeamNum);
			uintptr_t entity = *(uintptr_t*)((moduleBase + dwEntityList) + i * 0x10);//not broke but checking

			if (entity != NULL && localplayer != NULL && GetLocalPlayerHealth() >= 1) {//check if alivc
				bool entitydormant = *(bool*)(entity + m_bDormant);

				int entityHP = *(int*)((entity + m_iHealth));

				int entityTeam = *(int*)((entity + m_iTeamNum));

				Vector3 entityVectorOrigin = *(Vector3*)((entity + m_vecOrigin));
				Vector3 localplayerVectorOrigin = *(Vector3*)((localplayer + m_vecOrigin));

				int entityBoneMatrix = *(int*)((entity + m_dwBoneMatrix));
				Vector3 bonePos;
				bonePos.x = *(float*)(entityBoneMatrix + 0x30 * 8 + 0x0C);
				bonePos.y = *(float*)(entityBoneMatrix + 0x30 * 8 + 0x1C);
				bonePos.z = *(float*)(entityBoneMatrix + 0x30 * 8 + 0x2C);

				
				Vector3 bonePos7;
				bonePos7.x = *(float*)(entityBoneMatrix + 0x30 * 7 + 0x0C);
				bonePos7.y = *(float*)(entityBoneMatrix + 0x30 * 7 + 0x1C);
				bonePos7.z = *(float*)(entityBoneMatrix + 0x30 * 7 + 0x2C);


				//Sleep(1000);
				//std::cout << "0\n";
				if (entitydormant == true || entityHP <= 0 || entity == localplayer || entity == NULL) {// dead
				 //std::cout << "0.1\n";

					continue;
				}



				D3DCOLOR color;
				if (entityTeam == localplayerTeam) {
					color = D3DCOLOR_ARGB(255, 0, 255, 0);
					//std::cout << "1\n";
				}
				else {
					color = D3DCOLOR_ARGB(255, 255, 0, 0);
					//std::cout << "2\n";
				}
				
				if (localplayer != NULL) {
					DWORD localplayerHP = *(DWORD*)(localplayer + m_iHealth);
					DWORD LifeState = *(DWORD*)(localplayer + m_lifeState);

					Vector3 entHead3D = bonePos;
					Vector3 TBD = bonePos7;
					entHead3D.z = entHead3D.z +  8;
					Vector2 entPos2D, entHead2D;
					Vector2 TBD2D;
					if (LifeState == 0 || localplayerHP >= 1) {
						if (WorldToScreen(entityVectorOrigin, entPos2D)) {
							if (toggleSnapLines == true && localplayer != false && localplayerHP >= 1) {
								DrawLine(entPos2D.x, entPos2D.y, windowWidth / 2, windowHeight * 1.1, 2, color);
								//DrawLine(entPos2D.x, entPos2D.y, windowWidth, windowHeight * 2, 2, color);
							}
							/*
							if (toggleSnapLines == true && localplayer != false && localplayerHP >= 1) {
								if (WorldToScreen(entHead3D, entHead2D) && (WorldToScreen(TBD, TBD2D))) {
									DrawLine(entHead2D.x, entHead2D.y, TBD2D.x, TBD2D.y, 2, color);
								}
								//DrawLine(bonePos.x, bonePos.y, bonePos7.x, bonePos7.y, 2, color);
								
							}
							*/

							//DrawLine(windowWidth, windowHeight * 2, entPos2D.x, entPos2D.y, 2, color);
							//DrawLine(windowWidth * 0.5f, windowHeight, entPos2D.x, entPos2D.y, 2, color);
							//DrawLine(entPos2D.x, entPos2D.y, windowWidth * 0.5f, windowHeight, 2, color);
							// 
							// 
							//DrawFilledRect(entPos2D.x, entPos2D.y, 4, 4, D3DCOLOR_ARGB(255, 255, 255, 255));

							DWORD localplayerHP = *(DWORD*)(localplayer + m_iHealth);

							DWORD LifeState = *(DWORD*)(localplayer + m_lifeState);
							if (LifeState != 0 || localplayerHP < 1) {//player is dead
								continue;
							}
							if (LifeState == 0 || localplayerHP >= 1) {
								if (WorldToScreen(entHead3D, entHead2D)) {
									if (toggleBoxESP == true) {
										DrawEspBox2D(entPos2D, entHead2D, 2, color);
									}
								}
							}
						}
					}
				}
			}

		}

	}
	//std::cout << "loop ended\n";
	DrawFilledRect(windowWidth / 2 - 2, windowHeight / 2 - 2, 4, 4, D3DCOLOR_ARGB(255, 255, 255, 255));

	
	oEndScene(pDevice);
}








DWORD WINAPI MainThread(HMODULE hModule) {

		entity LocalPlayer;
		world GameWorld;
			
		bool toggleaimbot = true;
		int aimbotBone = 9;
		float toggleaimbotRCS = 1.00f;
		float aimbotSmoothing = 1.00f;

		//AllocConsole();																							 //DEBUG
		//FILE* f;																									 //DEBUG
		//freopen_s(&f, "CONOUT$", "w", stdout);                                                                     //DEBUG
		//HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);														 //DEBUG
		//SetConsoleTextAttribute(hConsole, 10);																   	 //DEBUG

		moduleBase = (DWORD)GetModuleHandle("client.dll");
		uint32_t engineModule = (uint32_t)GetModuleHandle("engine.dll");


		if (GetD3D9Device(d3d9Device, sizeof(d3d9Device))) {
			memcpy(EndSceneBytes, (char*)d3d9Device[42], 7);
			oEndScene = (tEndScene)TrampHook((char*)d3d9Device[42], (char*)hkEndScene, 7);
		}

		float ClosestEnt = 999999.f;
		int closest = -1;

		while (true) {

			Sleep(1);
			if (GetLocalPlayerHealth() < 1) {
				continue;
			}

			
			

			DWORD localplayer = *(DWORD*)(moduleBase + dwLocalPlayer);
			
			/*
			if (localplayer != NULL) {//check if not in menu
				DWORD LifeState = *(DWORD*)(localplayer + m_lifeState);
				DWORD localplayerHP = *(DWORD*)(localplayer + m_iHealth);
				if (localplayerHP < 1 || localplayerHP > 100 || LifeState == 0) {//dead check
					bool toggleGlowEsp = false;
					bool ShowMenu = false;
					bool toggleBhop = false;
					bool toggleTriggerBot = false;
					bool toggleSnapLines = false;
					bool toggleBoxESP = false;
				}
			}

			*/

			if (localplayer != NULL || GetLocalPlayerHealth() >= 1) {//alive


				DWORD localplayerHP = *(DWORD*)(localplayer + m_iHealth);
				DWORD GlowObjectManager = *(DWORD*)(moduleBase + dwGlowObjectManager);
				if (GetLocalPlayerHealth() < 1) {
					continue;
				}
				DWORD EntityList = *(DWORD*)(moduleBase + dwEntityList);
				DWORD LifeState = *(DWORD*)(localplayer + m_lifeState);

				LocalPlayer.EntityPtr = *(DWORD*)(moduleBase + dwLocalPlayer);

				if (LifeState != 0) {//dead
					continue;
				}
				if (GetLocalPlayerHealth() >= 1 && LifeState == 0) {//alive
					Update();
				}

				//uintptr_t localplayer = *(uintptr_t*)(moduleBase + dwLocalPlayer);
				//float flashDuration = *(float*)(localplayer + m_flFlashDuration);
				
				//int flashDur = flashDuration;
				//std::cout << flashDuration;

				//if (flashDuration != 0.f || flashDuration > 0.f) {
					//flashDuration = 0.f;
				//}


				if (GetAsyncKeyState(VK_END) & 1) {
					break;
				}

				

				if (localplayer != NULL) {

					if (LifeState == 0 || localplayerHP >= 1) {//alive
						if (LifeState != 0 || localplayerHP < 1) {//dead
							continue;
						}
						//   std::cout << "after if alive\n";

						   //if (LocalPlayer.EntityPtr != NULL) {

						if (LifeState != 0 || localplayerHP < 1) {//player is dead
							continue;
						}

						int CrosshairID = getCrosshairID(getLocalPlayer());
						int CrosshairTeam = getTeam(getPlayer(CrosshairID - 1));
						if (LifeState != 0 || localplayerHP < 1) {//player is dead
							continue;
						}
						int LocalTeam = getTeam(getLocalPlayer());

						if (LifeState != 0 || localplayerHP < 1) {//player is dead
							continue;
						}

						if (toggleTriggerBot == true) {

							if (CrosshairID > 0 && CrosshairID < 32 && LocalTeam != CrosshairTeam) {
								mouse_event(MOUSEEVENTF_LEFTDOWN, NULL, NULL, 0, 0);
								mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, 0, 0);
								
								Sleep(1);
							}
						}

						
						if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0) {


							//std::cout << "V Pressed ";
							
							//uint32_t engineModule = (uint32_t)GetModuleHandle("engine.dll");
							int maxPlayers = (int)(*(uint32_t*)(engineModule + dwClientState_MaxPlayer));

							//std::cout << maxPlayers;
							
							for (int i = 1; i < 32; i++) {
								DWORD localplayer = *(DWORD*)(moduleBase + dwLocalPlayer);

								DWORD entity = *(DWORD*)((moduleBase + dwEntityList) + i * 0x10);
								if (entity != NULL) {



									bool entitydormant = *(bool*)(entity + m_bDormant);
									int entityTeam = *(int*)((entity + m_iTeamNum));
									int localplayerTeam = *(int*)((localplayer + m_iTeamNum));
									int entityHealth = *(int*)((entity + m_iHealth));

									//std::cout << i << "f";

									if (entity != NULL && entityTeam != localplayerTeam && entitydormant == false && entityHealth >= 1 || entityHealth > 100) {//if enemy and alive
										Vector3 localplayerVectorOrigin = *(Vector3*)(localplayer + m_vecOrigin);
										Vector3 entityplayerVectorOrigin = *(Vector3*)(entity + m_vecOrigin);
										Vector3 deltaVec = { entityplayerVectorOrigin.x - localplayerVectorOrigin.x,entityplayerVectorOrigin.y - localplayerVectorOrigin.y, entityplayerVectorOrigin.z - localplayerVectorOrigin.z };
										float deltaSQRT = sqrt(deltaVec.x * deltaVec.x + deltaVec.y * deltaVec.y + deltaVec.z * deltaVec.z);
										float DistanceFromEnemy = deltaSQRT;

										if (DistanceFromEnemy < ClosestEnt) {
											ClosestEnt = DistanceFromEnemy;
											closest = i;
										}
									}
								
								}
							}

							DWORD closestEntity = *(DWORD*)((moduleBase + dwEntityList) + closest * 0x10);
							Vector3 closestEntityplayerVectorOrigin = *(Vector3*)(closestEntity + m_vecOrigin);
							Vector3 newLocalplayerVectorOrigin = *(Vector3*)(localplayer + m_vecOrigin);

							uint32_t entityBoneMatrix = *(uint32_t*)((closestEntity + m_dwBoneMatrix));

							//Vector3 entitybonePos;
							//entitybonePos.x = *(float*)(entityBoneMatrix + 0x30 * 8 + 0x0C);
							//entitybonePos.y = *(float*)(entityBoneMatrix + 0x30 * 8 + 0x1C);
							//entitybonePos.z = *(float*)(entityBoneMatrix + 0x30 * 8 + 0x2C);

							//Vector3 entHead3D = entitybonePos;
							//Vector2 entPos2D, entHead2D;

							//if (WorldToScreen(entHead3D, entHead2D)) {



								Vector3 newDeltaVec = { closestEntityplayerVectorOrigin.x - newLocalplayerVectorOrigin.x,closestEntityplayerVectorOrigin.y - newLocalplayerVectorOrigin.y, closestEntityplayerVectorOrigin.z - newLocalplayerVectorOrigin.z };
								//Vector3 newDeltaVec = { entHead2D.x - newLocalplayerVectorOrigin.x,entHead2D.y - newLocalplayerVectorOrigin.y, closestEntityplayerVectorOrigin.z - newLocalplayerVectorOrigin.z };

								float deltaVecLength = sqrt(newDeltaVec.x * newDeltaVec.x + newDeltaVec.y * newDeltaVec.y + newDeltaVec.z * newDeltaVec.z);

								float pitch = -asin(newDeltaVec.z / deltaVecLength) * (180 / 3.14159265358 + 5);
								float yaw = atan2(newDeltaVec.y, newDeltaVec.x) * (180 / 3.14159265358);



								Vector3* viewAngles = (Vector3*)(*(uint32_t*)(engineModule + dwClientState) + dwClientState_ViewAngles);


								viewAngles->x = pitch;
								viewAngles->y = yaw;

								ClosestEnt = 999999.f;
								closest = -1;
							//}
						}

						if (LifeState != 0 || localplayerHP < 1) {//player is dead
							continue;
						}

						int localteam = *(int*)(localplayer + m_iTeamNum);

						if (LifeState != 0 || localplayerHP < 1) {//dead
							continue;
						}

						LocalPlayer.flags = *(int*)(LocalPlayer.EntityPtr + m_fFlags);


						if (LifeState != 0 || localplayerHP < 1) {//player is dead
							continue;
						}

						//     std::cout << "before ESP\n";
						if (toggleGlowEsp == true && localplayer != NULL && localplayerHP >= 1) {

							if (LifeState != 0 || localplayerHP < 1) {//dead
								continue;
							}

							for (int i = 1; i < 32; i++) {

								DWORD entity = *(DWORD*)((moduleBase + dwEntityList) + i * 0x10);

								//if (LifeState != 0 || localplayerHP < 1) {//dead
								//    continue;
								//}

								if (entity == NULL) continue;
								int glowIndex = *(int*)(entity + m_iGlowIndex);
								int entityTeam = *(int*)(entity + m_iTeamNum);

								//if (LifeState != 0 || localplayerHP < 1) {//dead
								//    continue;
								//}

								if (entityTeam == localteam) {

									*(float*)((GlowObjectManager + glowIndex * 0x38 + 0x8)) = 0.f; //R
									*(float*)((GlowObjectManager + glowIndex * 0x38 + 0xC)) = 1.f; //G

									//if (LifeState != 0 || localplayerHP < 1) {//dead
									//    continue;
									//}
									*(float*)((GlowObjectManager + glowIndex * 0x38 + 0x10)) = 0.f; //B
									*(float*)((GlowObjectManager + glowIndex * 0x38 + 0x14)) = 1.7f; //A

									//if (LifeState != 0 || localplayerHP < 1) {//dead
									//    continue;
									//}
								}
								else {
									*(float*)((GlowObjectManager + glowIndex * 0x38 + 0x8)) = 1.f; //R
									*(float*)((GlowObjectManager + glowIndex * 0x38 + 0xC)) = 0.f; //G

									//if (LifeState != 0 || localplayerHP < 1) {//dead
									//    continue;
									//}
									*(float*)((GlowObjectManager + glowIndex * 0x38 + 0x10)) = 0.f; //B
									*(float*)((GlowObjectManager + glowIndex * 0x38 + 0x14)) = 1.7f; //A
								}

								*(bool*)((GlowObjectManager + glowIndex * 0x38 + 0x28)) = true;
								*(bool*)((GlowObjectManager + glowIndex * 0x38 + 0x29)) = false;

								//if (LifeState != 0 || localplayerHP < 1) {//dead
								//    continue;
								//}

							}
						}

						if (LifeState != 0 || localplayerHP < 1) {//player is dead
							continue;
						}
						
						if (GetAsyncKeyState(VK_SPACE) && LocalPlayer.flags == 257 && toggleBhop == true) {

							LocalPlayer.ForceJump(GameWorld.GameModule);
							//std::cout << "HOP\n";
						}
						//std::cout << "after bhop\n";


						if (LifeState != 0 || localplayerHP < 1) {//player is dead
							continue;
						}

						if (GetAsyncKeyState(VK_F6)) {
							//SetCursorPos(-100, -100);
							//mouse_event(MOUSEEVENTF_MOVE, 10, 10, 0, NULL);


							for (int i = 1; i <= 32; i++) {
								uintptr_t entity = *(uintptr_t*)((moduleBase + dwEntityList) + i * 0x10);
								uintptr_t localplayer = *(uintptr_t*)(moduleBase + dwLocalPlayer);
								Vector3 entityVectorOrigin = *(Vector3*)((entity + m_vecOrigin));
								Vector3 localplayerVectorOrigin = *(Vector3*)((localplayer + m_vecOrigin));

								//Vector3 delta = (entityVectorOrigin.x - localplayerVectorOrigin.x, entityVectorOrigin.y - localplayerVectorOrigin.y, entityVectorOrigin.z - localplayerVectorOrigin.z);
							}
						}

					}
				}

			}
		}

		Patch((BYTE*)d3d9Device[42], EndSceneBytes, 7);
		//fclose(f);																						  //DEBUG
		//FreeConsole();																					//DEBUG
		//ImGui::End();
		
		ImGui_ImplDX9_Shutdown();//goodish
		ImGui::DestroyContext();//goodish
		FreeLibraryAndExitThread(hModule, 0);
		return 0;
	
}


BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr));

		//MessageBoxA(NULL, "DLL injected", "DLL injected", MB_OK);


	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
