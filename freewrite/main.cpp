#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <conio.h>
#include <Windows.h>
#include "hprocess.h"
#include "ofs.h"

using namespace std;
Hackprocess hEngine;

struct user {
	long long HWID;
}users[100];

struct cfg {
	struct triggerbot {
		int delay;
		int time;
	}triggerbot;
	struct aimassist {
		int bone;
		int smooth;
		int rcs;
	}aimassist;
	struct aimbot {
		int bone;
		int fov;
		int smooth;
		int rcs;
	}aimbot;
	struct misc {
		int aimkey;
		int aimmode;
		int aimassistkey;
		int aimassistmode;
		int triggerkey;
		int triggermode;
		int espactive;
		int visiblecheck;
	}misc;
}cfg[100];

struct GlowObjectDefinition
{
	DWORD pEntity;
	float r;
	float g;
	float b;
	float a;
	uint8_t unk1[16];
	bool m_bRenderWhenOccluded;
	bool m_bRenderWhenUnoccluded;
	bool m_bFullBloom;
	uint8_t unk2[14];
};

struct Player {
	DWORD PlayerBase;
	DWORD GlowBase;
	DWORD AnglePointer;
	DWORD WeaponEntityIndex;
	DWORD WeaponBase;
	int GlowCount;
	int Clip;
	int wn;
	int Team;
	int ShotsFired;
	int CrosshairID;
	byte Flags;
	float Punch[3];
	float Position[3];
	float Angles[3];
	bool ReadInformation() {
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(hEngine.dwordClient + dw_LocalPlayer), &PlayerBase, sizeof(DWORD), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(hEngine.dwordClient + GlowObject), &GlowBase, sizeof(DWORD), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(hEngine.dwordEngine + dw_EnginePointer), &AnglePointer, sizeof(DWORD), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(hEngine.dwordClient + GlowObject + 0x4), &GlowCount, sizeof(int), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(PlayerBase + i_TeamNumber), &Team, sizeof(int), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(PlayerBase + i_ShotsFired), &ShotsFired, sizeof(int), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(PlayerBase + i_CrosshairId), &CrosshairID, sizeof(int), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(PlayerBase + m_fFlags), &Flags, sizeof(byte), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(PlayerBase + vec_Punch), &Punch, sizeof(vector<float>), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(PlayerBase + vec_Origin), &Position, sizeof(vector<float>), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(AnglePointer + vec_ViewAngle), &Angles, sizeof(vector<float>), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(PlayerBase + dw_ActiveWeapon), &WeaponEntityIndex, sizeof(DWORD), NULL);
		WeaponEntityIndex &= 0xFFF;
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(hEngine.dwordClient + dw_EntityList + (WeaponEntityIndex * 0x10) - 0x10), &WeaponBase, sizeof(DWORD), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(WeaponBase + dw_WeaponID), &wn, sizeof(int), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(WeaponBase + m_iPrimaryAmmoType), &Clip, sizeof(int), NULL);
		if (Clip == 4 && wn == 16) {
			wn = 60;
		}
		if (Clip == 11 && wn == 32) {
			wn = 61;
		}
		return true;
	}
}Player;

struct Entity {
	DWORD EntityBase;
	DWORD BoneMatrix;
	bool Dormant;
	int Team;
	int HP;
	bool IsDead;
	bool Spot;
	float abBonePosition[3];
	float aaBonePosition[3];
	void ReadInformation(int nPlayer) {
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(hEngine.dwordClient + dw_EntityList + nPlayer * 0x10), &EntityBase, sizeof(DWORD), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(EntityBase + dw_BoneMatrix), &BoneMatrix, sizeof(DWORD), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(EntityBase + b_Dormant), &Dormant, sizeof(bool), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(EntityBase + i_TeamNumber), &Team, sizeof(int), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(EntityBase + i_HP), &HP, sizeof(int), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(EntityBase + b_LifeState), &IsDead, sizeof(bool), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(EntityBase + b_Spotted), &Spot, sizeof(bool), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(BoneMatrix + 0x30 * cfg[Player.wn].aimbot.bone + 0x0C), &abBonePosition[0], sizeof(float), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(BoneMatrix + 0x30 * cfg[Player.wn].aimbot.bone + 0x1C), &abBonePosition[1], sizeof(float), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(BoneMatrix + 0x30 * cfg[Player.wn].aimbot.bone + 0x2C), &abBonePosition[2], sizeof(float), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(BoneMatrix + 0x30 * cfg[Player.wn].aimassist.bone + 0x0C), &aaBonePosition[0], sizeof(float), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(BoneMatrix + 0x30 * cfg[Player.wn].aimassist.bone + 0x1C), &aaBonePosition[1], sizeof(float), NULL);
		ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(BoneMatrix + 0x30 * cfg[Player.wn].aimassist.bone + 0x2C), &aaBonePosition[2], sizeof(float), NULL);
		return;
	}
}Entity[32];

void ReadConfig() {
	ifstream in("config.txt");
	string Value, NameWeapon, SetWeapons, SetMain, SetEsp;
	in >> SetMain;
	in >> Value >> cfg[0].misc.aimkey;
	in >> Value >> cfg[0].misc.aimmode;
	in >> Value >> cfg[0].misc.aimassistkey;
	in >> Value >> cfg[0].misc.aimassistmode;
	in >> Value >> cfg[0].misc.triggerkey;
	in >> Value >> cfg[0].misc.triggermode;
	in >> Value >> cfg[0].misc.espactive;
	in >> Value >> cfg[0].misc.visiblecheck;
	in >> SetWeapons;
	for (int i = 1; i <= 4; i++) {
		in >> NameWeapon;
		in >> Value >> cfg[i].triggerbot.delay;
		in >> Value >> cfg[i].triggerbot.time;
		in >> Value >> cfg[i].aimassist.bone;
		in >> Value >> cfg[i].aimassist.smooth;
		in >> Value >> cfg[i].aimassist.rcs;
		in >> Value >> cfg[i].aimbot.bone;
		in >> Value >> cfg[i].aimbot.fov;
		in >> Value >> cfg[i].aimbot.smooth;
		in >> Value >> cfg[i].aimbot.rcs;
	}
	for (int i = 7; i <= 11; i++) {
		in >> NameWeapon;
		in >> Value >> cfg[i].triggerbot.delay;
		in >> Value >> cfg[i].triggerbot.time;
		in >> Value >> cfg[i].aimassist.bone;
		in >> Value >> cfg[i].aimassist.smooth;
		in >> Value >> cfg[i].aimassist.rcs;
		in >> Value >> cfg[i].aimbot.bone;
		in >> Value >> cfg[i].aimbot.fov;
		in >> Value >> cfg[i].aimbot.smooth;
		in >> Value >> cfg[i].aimbot.rcs;
	}
	for (int i = 13; i <= 14; i++) {
		in >> NameWeapon;
		in >> Value >> cfg[i].triggerbot.delay;
		in >> Value >> cfg[i].triggerbot.time;
		in >> Value >> cfg[i].aimassist.bone;
		in >> Value >> cfg[i].aimassist.smooth;
		in >> Value >> cfg[i].aimassist.rcs;
		in >> Value >> cfg[i].aimbot.bone;
		in >> Value >> cfg[i].aimbot.fov;
		in >> Value >> cfg[i].aimbot.smooth;
		in >> Value >> cfg[i].aimbot.rcs;
	}
	for (int i = 16; i <= 17; i++) {
		in >> NameWeapon;
		in >> Value >> cfg[i].triggerbot.delay;
		in >> Value >> cfg[i].triggerbot.time;
		in >> Value >> cfg[i].aimassist.bone;
		in >> Value >> cfg[i].aimassist.smooth;
		in >> Value >> cfg[i].aimassist.rcs;
		in >> Value >> cfg[i].aimbot.bone;
		in >> Value >> cfg[i].aimbot.fov;
		in >> Value >> cfg[i].aimbot.smooth;
		in >> Value >> cfg[i].aimbot.rcs;
	}
	for (int i = 19; i <= 19; i++) {
		in >> NameWeapon;
		in >> Value >> cfg[i].triggerbot.delay;
		in >> Value >> cfg[i].triggerbot.time;
		in >> Value >> cfg[i].aimassist.bone;
		in >> Value >> cfg[i].aimassist.smooth;
		in >> Value >> cfg[i].aimassist.rcs;
		in >> Value >> cfg[i].aimbot.bone;
		in >> Value >> cfg[i].aimbot.fov;
		in >> Value >> cfg[i].aimbot.smooth;
		in >> Value >> cfg[i].aimbot.rcs;
	}
	for (int i = 24; i <= 36; i++) {
		in >> NameWeapon;
		in >> Value >> cfg[i].triggerbot.delay;
		in >> Value >> cfg[i].triggerbot.time;
		in >> Value >> cfg[i].aimassist.bone;
		in >> Value >> cfg[i].aimassist.smooth;
		in >> Value >> cfg[i].aimassist.rcs;
		in >> Value >> cfg[i].aimbot.bone;
		in >> Value >> cfg[i].aimbot.fov;
		in >> Value >> cfg[i].aimbot.smooth;
		in >> Value >> cfg[i].aimbot.rcs;
	}
	for (int i = 38; i <= 40; i++) {
		in >> NameWeapon;
		in >> Value >> cfg[i].triggerbot.delay;
		in >> Value >> cfg[i].triggerbot.time;
		in >> Value >> cfg[i].aimassist.bone;
		in >> Value >> cfg[i].aimassist.smooth;
		in >> Value >> cfg[i].aimassist.rcs;
		in >> Value >> cfg[i].aimbot.bone;
		in >> Value >> cfg[i].aimbot.fov;
		in >> Value >> cfg[i].aimbot.smooth;
		in >> Value >> cfg[i].aimbot.rcs;
	}
	for (int i = 60; i <= 61; i++) {
		in >> NameWeapon;
		in >> Value >> cfg[i].triggerbot.delay;
		in >> Value >> cfg[i].triggerbot.time;
		in >> Value >> cfg[i].aimassist.bone;
		in >> Value >> cfg[i].aimassist.smooth;
		in >> Value >> cfg[i].aimassist.rcs;
		in >> Value >> cfg[i].aimbot.bone;
		in >> Value >> cfg[i].aimbot.fov;
		in >> Value >> cfg[i].aimbot.smooth;
		in >> Value >> cfg[i].aimbot.rcs;
	}
	for (int i = 63; i <= 64; i++) {
		in >> NameWeapon;
		in >> Value >> cfg[i].triggerbot.delay;
		in >> Value >> cfg[i].triggerbot.time;
		in >> Value >> cfg[i].aimassist.bone;
		in >> Value >> cfg[i].aimassist.smooth;
		in >> Value >> cfg[i].aimassist.rcs;
		in >> Value >> cfg[i].aimbot.bone;
		in >> Value >> cfg[i].aimbot.fov;
		in >> Value >> cfg[i].aimbot.smooth;
		in >> Value >> cfg[i].aimbot.rcs;
	}
	in.close();
	return;
}

void CalcAngles(float *ebp, float *pp, float *ang, int rcs) {
	float delta[3] = { ebp[0] - pp[0], ebp[1] - pp[1], ebp[2] - (pp[2] + 61) };
	float dist = sqrt((delta[0] * delta[0]) + (delta[1] * delta[1]) + (delta[2] * delta[2]));
	float hyp = sqrt((delta[0] * delta[0] + delta[1] * delta[1]));
	float hAng = acos(delta[0] / hyp) * pfx;
	if (delta[1] < 0) {
		hAng *= -1;
	}
	float vAng = acos(delta[2] / dist) * pfx - 90;
	ang[0] = vAng - Player.Punch[0] * 2.0f * rcs;
	ang[1] = hAng - Player.Punch[1] * 2.0f * rcs;
	ang[2] = 0;
	return;
}

void SmoothAngles(float *ang, float smooth) {
	if (smooth > 0) {
		float a_fixAng = ang[1], p_fixAng = Player.Angles[1], distance = 0;
		if (ang[1] < 0) {
			a_fixAng = 360 + ang[1];
		}
		if (Player.Angles[1] < 0) {
			p_fixAng = 360 + Player.Angles[1];
		}
		distance = min(abs(a_fixAng - p_fixAng), 360 - abs(a_fixAng - p_fixAng));
		if (Player.Angles[1] > 90 && ang[1] < -90) {
			ang[0] = Player.Angles[0] + (ang[0] - Player.Angles[0]) / smooth;
			ang[1] = Player.Angles[1] + distance / smooth;
			if (ang[1] > 180.f) {
				ang[1] = -360 + ang[1];
			}
			return;
		}
		if (Player.Angles[1] < -90 && ang[1] > 90) {
			ang[0] = Player.Angles[0] + (ang[0] - Player.Angles[0]) / smooth;
			ang[1] = Player.Angles[1] - distance / smooth;
			if (ang[1] < -180.f) {
				ang[1] = 360 + ang[1];
			}
			return;
		}
		ang[0] = Player.Angles[0] + (ang[0] - Player.Angles[0]) / smooth;
		ang[1] = Player.Angles[1] + (ang[1] - Player.Angles[1]) / smooth;
	}
	return;
}

void AngleNormalize(float* angles)
{
	if (angles[0] > 89.0f && angles[0] <= 180.0f)
	{
		angles[0] = 89.0f;
	}
	if (angles[0] > 180.f)
	{
		angles[0] -= 360.f;
	}
	if (angles[0] < -89.0f)
	{
		angles[0] = -89.0f;
	}
	if (angles[1] > 180.f)
	{
		angles[1] -= 360.f;
	}
	if (angles[1] < -180.f)
	{
		angles[1] += 360.f;
	}
	if (angles[2] != 0.0f)
	{
		angles[2] = 0.0f;
	}
	return;
}

bool ValidTarget(int vTarget, int vFov) {
	if (vTarget == 0) {
		return false;
	}
	if (vTarget > 32) {
		return false;
	}
	if (Entity[vTarget].Team == Player.Team) {
		return false;
	}
	if (Entity[vTarget].IsDead) {
		return false;
	}
	if (Entity[vTarget].Dormant) {
		return false;
	}
	if (!Entity[vTarget].Spot) {
		if (cfg[0].misc.visiblecheck == 1) {
			return false;
		}
	}
	if (Entity[vTarget].HP <= 0 || Entity[vTarget].HP > 100) {
		return false;
	}
	if (vFov <= cfg[Player.wn].aimbot.fov) {
		return true;
	}
	return false;
}

int BestTarget() {
	int Target = -1;
	float mFov = 360;
	for (int i = 1; i < 32; i++) {
		float Angles[3];
		CalcAngles(Entity[i].abBonePosition, Player.Position, Angles, cfg[Player.wn].aimbot.rcs);
		float tFov, hAng, vAng, a_fixAng = Angles[1], p_fixAng = Player.Angles[1];
		vAng = Angles[0] - Player.Angles[0];
		if (Angles[1] < 0) {
			a_fixAng = 360 + Angles[1];
		}
		if (Player.Angles[1] < 0) {
			p_fixAng = 360 + Player.Angles[1];
		}
		hAng = min(abs(a_fixAng - p_fixAng), 360 - abs(a_fixAng - p_fixAng));
		tFov = sqrt(hAng * hAng + vAng * vAng);
		if (tFov < mFov && ValidTarget(i, tFov)) {
			mFov = tFov;
			Target = i;
		}
	}
	return Target;
}

void esp() {
	if (cfg[0].misc.espactive == 1) {
		if (Player.GlowBase != NULL) {
			for (int i = 0; i < Player.GlowCount; i++) {
				DWORD mObj = Player.GlowBase + i * sizeof(GlowObjectDefinition);
				GlowObjectDefinition glowObj;
				ReadProcessMemory(hEngine.HandleProcess, (PBYTE*)(mObj), &glowObj, sizeof(GlowObjectDefinition), NULL);
				if (glowObj.pEntity != NULL) {
					for (int j = 1; j < 32; j++) {
						float red = 0, green = 0, blue = 0;
						if (glowObj.pEntity == Entity[j].EntityBase) {
							float a = 1.0f;
							if (!Entity[j].Dormant && Entity[j].HP > 0) {
								if (Entity[j].Team == 3) {
									blue = 1;
								}
								else {
									red = 1;
								}
								WriteProcessMemory(hEngine.HandleProcess, (PBYTE*)(mObj + 0x4), &red, sizeof(float), NULL);
								WriteProcessMemory(hEngine.HandleProcess, (PBYTE*)(mObj + 0x8), &green, sizeof(float), NULL);
								WriteProcessMemory(hEngine.HandleProcess, (PBYTE*)(mObj + 0xC), &blue, sizeof(float), NULL);
								WriteProcessMemory(hEngine.HandleProcess, (PBYTE*)(mObj + 0x10), &a, sizeof(float), NULL);
								WriteProcessMemory(hEngine.HandleProcess, (PBYTE*)(mObj + 0x24), &b_True, sizeof(bool), NULL);
								WriteProcessMemory(hEngine.HandleProcess, (PBYTE*)(mObj + 0x25), &b_False, sizeof(bool), NULL);
							}
						}
					}
				}
			}
		}
	}
	return;
}

void triggerbot() {
	if (cfg[0].misc.triggermode == 0) {
		return;
	}
	if (cfg[0].misc.triggermode == 2 && !GetAsyncKeyState(cfg[0].misc.triggerkey)) {
		return;
	}
	if (Player.CrosshairID == 0) {
		return;
	}
	if (Player.CrosshairID > 32) {
		return;
	}
	if (Entity[Player.CrosshairID - 1].Team == Player.Team) {
		return;
	}
	if (Player.CrosshairID > 0 && Player.CrosshairID < 32) {
		Sleep(cfg[Player.wn].triggerbot.delay);
		WriteProcessMemory(hEngine.HandleProcess, (PBYTE*)(hEngine.dwordClient + dw_Atack), &b_True, sizeof(bool), NULL);
		Sleep(cfg[Player.wn].triggerbot.time);
		WriteProcessMemory(hEngine.HandleProcess, (PBYTE*)(hEngine.dwordClient + dw_Atack), &b_False, sizeof(bool), NULL);
	}
	return;
}

void aimassist() {
	if (cfg[0].misc.aimassistmode == 0) {
		return;
	}
	if (!GetAsyncKeyState(cfg[0].misc.aimassistkey) && cfg[0].misc.aimassistmode == 2) {
		return;
	}
	int Target = Player.CrosshairID - 1;
	if (Target != -1) {
		float Angles[3]; Angles[0] = Player.Angles[0]; Angles[1] = Player.Angles[1]; Angles[2] = 0;
		CalcAngles(Entity[Target].aaBonePosition, Player.Position, Angles, cfg[Player.wn].aimassist.rcs);
		SmoothAngles(Angles, cfg[Player.wn].aimassist.smooth);
		AngleNormalize(Angles);
		if (Angles[1] < 180.0f && Angles[1] > -180.0f && Angles[0] < 90.0f && Angles[0] > -90.0f) {
			WriteProcessMemory(hEngine.HandleProcess, (PBYTE*)(Player.AnglePointer + vec_ViewAngle), &Angles, sizeof(vector<float>), NULL);
		}
	}
	return;
}

void aimbot() {
	if (cfg[0].misc.aimmode == 0) {
		return;
	}
	if (cfg[0].misc.aimmode == 2 && !GetAsyncKeyState(cfg[0].misc.aimkey)) {
		return;
	}
	int Target = BestTarget();
	int work = Player.ShotsFired;
	if (Target != -1) {
		float Angles[3]; Angles[0] = Player.Angles[0]; Angles[1] = Player.Angles[1]; Angles[2] = 0;
		CalcAngles(Entity[Target].abBonePosition, Player.Position, Angles, cfg[Player.wn].aimbot.rcs);
		SmoothAngles(Angles, cfg[Player.wn].aimbot.smooth);
		AngleNormalize(Angles);
		if (Angles[1] < 180.0f && Angles[1] > -180.0f && Angles[0] < 90.0f && Angles[0] > -90.0f) {
			WriteProcessMemory(hEngine.HandleProcess, (PBYTE*)(Player.AnglePointer + vec_ViewAngle), &Angles, sizeof(vector<float>), NULL);
		}
	}
	return;
}

void WELLCOME() {
	system("title HACK");
	cout << "                          SHIT-HACK FOR CSGO" << endl;
	return;
}

int main() {
	WELLCOME();
	ReadConfig();
	hEngine.memoryType();
	while (true) {
		for (int i = 0; i < 32; i++) {
			Entity[i].ReadInformation(i);
		}
		Player.ReadInformation();
		esp();
		triggerbot();
		aimassist();
		aimbot();
		Sleep(1);
	}
	return 0;
}
