#pragma once
#include <intrin.h>
#include <DirectXMath.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <Windows.h>
#include "../utils/xorstr.hpp"

constexpr float M_PI = 3.1415926535f;
constexpr size_t MAX_PLAYERS = 64;
constexpr size_t MAX_ITEMS = 1024;

namespace enums
{
    enum Team : short
    {
        Unknown = 0,
        Spectator = 1,
        Terrorists = 2,
        CounterTerrorists = 3
    };

    enum ItemDefinitionIndex : short int
    {
		WeaponDeagle = 1,
		WeaponElite = 2,
		WeaponFiveseven = 3,
		WeaponGlock = 4,
		WeaponAk47 = 7,
		WeaponAug = 8,
		WeaponAwp = 9,
		WeaponFamas = 10,
		WeaponG3sg1 = 11,
		WeaponGalilar = 13,
		WeaponM249 = 14,
		WeaponM4a1 = 16,
		WeaponMac10 = 17,
		WeaponP90 = 19,
		WeaponMp5sd = 23,
		WeaponUmp45 = 24,
		WeaponXm1014 = 25,
		WeaponBizon = 26,
		WeaponMag7 = 27,
		WeaponNegev = 28,
		WeaponSawedoff = 29,
		WeaponTec9 = 30,
		WeaponTaser = 31,
		WeaponHkp2000 = 32,
		WeaponMp7 = 33,
		WeaponMp9 = 34,
		WeaponNova = 35,
		WeaponP250 = 36,
		WeaponScar20 = 38,
		WeaponSg556 = 39,
		WeaponSsg08 = 40,
		WeaponKnifegg = 41,
		WeaponKnife = 42,
		WeaponFlashbang = 43,
		WeaponHegrenade = 44,
		WeaponSmokegrenade = 45,
		WeaponMolotov = 46,
		WeaponDecoy = 47,
		WeaponIncgrenade = 48,
		WeaponC4 = 49,
		ItemKevlar = 50,
		ItemAssaultsuit = 51,
		ItemHeavyassaultsuit = 52,
		ItemNvg = 54,
		ItemDefuser = 55,
		ItemCutters = 56,
		WeaponHealthshot = 57,
		MusickitDefault = 58,
		WeaponKnifeT = 59,
		WeaponM4a1Silencer = 60,
		WeaponUspSilencer = 61,
		WeaponCz75a = 63,
		WeaponRevolver = 64,
		WeaponTagrenade = 68,
		WeaponFists = 69,
		WeaponBreachcharge = 70,
		WeaponTablet = 72,
		WeaponMelee = 74,
		WeaponAxe = 75,
		WeaponHammer = 76,
		WeaponSpanner = 78,
		WeaponKnifeGhost = 80,
		WeaponFirebomb = 81,
		WeaponDiversion = 82,
		WeaponFragGrenade = 83,
		WeaponSnowball = 84,
		WeaponBayonet = 500,
		WeaponKnifeFlip = 505,
		WeaponKnifeGut = 506,
		WeaponKnifeKarambit = 507,
		WeaponKnifeM9Bayonet = 508,
		WeaponKnifeTactical = 509,
		WeaponKnifeFalchion = 512,
		WeaponKnifeSurvivalBowie = 514,
		WeaponKnifeButterfly = 515,
		WeaponKnifePush = 516,
		WeaponKnifeUrsus = 519,
		WeaponKnifeGypsyJackknife = 520,
		WeaponKnifeStiletto = 522,
		WeaponKnifeWidowmaker = 523,
		Musickit = 1314,
		StuddedBloodhoundGloves = 5027,
		TGloves = 5028,
		CtGloves = 5029,
		SportyGloves = 5030,
		SlickGloves = 5031,
		LeatherHandwraps = 5032,
		MotorcycleGloves = 5033,
		SpecialistGloves = 5034,
		StuddedHydraGloves = 5035,
    };

	inline std::unordered_map<ItemDefinitionIndex, int> MaxClipAmmo = {
	{WeaponAk47, 30},
	{WeaponM4a1, 30},
	{WeaponM4a1Silencer, 20},
	{WeaponAwp, 5},
	{WeaponDeagle, 7},
	{WeaponGlock, 20},
	{WeaponUspSilencer, 12},
	{WeaponHkp2000, 13},
	{WeaponFiveseven, 20},
	{WeaponElite, 30},
	{WeaponP250, 13},
	{WeaponCz75a, 12},
	{WeaponTec9, 18},
	{WeaponRevolver, 8},
	{WeaponAug, 30},
	{WeaponFamas, 25},
	{WeaponGalilar, 35},
	{WeaponM249, 100},
	{WeaponNegev, 150},
	{WeaponSg556, 30},
	{WeaponScar20, 20},
	{WeaponG3sg1, 20},
	{WeaponSsg08, 10},
	{WeaponBizon, 64},
	{WeaponMac10, 30},
	{WeaponMp5sd, 30},
	{WeaponMp7, 30},
	{WeaponMp9, 30},
	{WeaponP90, 50},
	{WeaponUmp45, 25},
	{WeaponMag7, 5},
	{WeaponNova, 8},
	{WeaponSawedoff, 7},
	{WeaponXm1014, 7},
	// Grenades
	{WeaponFlashbang, 0},
	{WeaponHegrenade, 0},
	{WeaponSmokegrenade, 0},
	{WeaponMolotov, 0},
	{WeaponDecoy, 0},
	{WeaponIncgrenade, 0},
	{WeaponTagrenade, 0},
	{WeaponFirebomb, 0},
	{WeaponDiversion, 0},
	{WeaponFragGrenade, 0},
	{WeaponSnowball, 0},
	{WeaponHealthshot, 1},
	{WeaponTaser, 1},
	{WeaponBreachcharge, 0},
	{ItemDefuser, 0},
	};

	inline std::unordered_map<ItemDefinitionIndex, char> ItemsNames = {
			{WeaponKnifeFalchion, '0'},
			{WeaponBayonet, '1'},
			{WeaponKnifeFlip, '2'},
			{WeaponKnifeGut, '3'},
			{WeaponKnifeKarambit, '4'},
			{WeaponKnifeM9Bayonet, '5'},
			{WeaponKnifeButterfly, '8'},
			{WeaponKnifePush, '9'},
			{WeaponDeagle, 'A'},
			{WeaponElite, 'B'},
			{WeaponFiveseven, 'C'},
			{WeaponGlock, 'D'},
			{WeaponHkp2000, 'E'},
			{WeaponP250, 'F'},
			{WeaponUspSilencer, 'G'},
			{WeaponTec9, 'H'},
			{WeaponCz75a, 'I'},
			{WeaponRevolver, 'J'},
			{WeaponMac10, 'K'},
			{WeaponUmp45, 'L'},
			{WeaponBizon, 'M'},
			{WeaponMp7, 'N'},
			{WeaponMp9, 'O'},
			{WeaponIncgrenade, 'n'},
			{WeaponP90, 'P'},
			{WeaponGalilar, 'Q'},
			{WeaponFamas, 'R'},
			{WeaponM4a1, 'S'},
			{WeaponM4a1Silencer, 'T'},
			{WeaponAug, 'U'},
			{WeaponSg556, 'V'},
			{WeaponAk47, 'W'},
			{WeaponG3sg1, 'X'},
			{WeaponScar20, 'Y'},
			{WeaponAwp, 'Z'},
			{WeaponKnife, ']'},
			{WeaponKnifeT, '['},
			{WeaponSsg08, 'a'},
			{WeaponXm1014, 'b'},
			{WeaponSawedoff, 'c'},
			{WeaponMag7, 'd'},
			{WeaponNova, 'e'},
			{WeaponNegev, 'f'},
			{WeaponM249, 'g'},
			{WeaponTaser, 'h'},
			{WeaponHegrenade, 'j'},
			{WeaponFlashbang, 'i'},
			{WeaponFragGrenade, 'j'},
			{WeaponSmokegrenade, 'k'},
			{WeaponMolotov, 'l'},
			{WeaponDecoy, 'm'},
			{WeaponFirebomb, 'n'},
			{WeaponC4, 'o'},
			{WeaponMp5sd, 'x'},
			{ItemDefuser, 'r'},
	};

	inline std::unordered_map<std::string, std::string> ItemsProjectile = {
		{xorstr_("smokegrenade_projectile"), "k"},
		{xorstr_("hegrenade_projectile"), "j"},
		{xorstr_("molotov_projectile"), "l"},
		{xorstr_("flashbang_projectile"), "i"},
		{xorstr_("decoy_projectile"), "m"},
	};
}

namespace structures
{
	template<typename T, size_t MaxSize>
	class FixedArray {
	private:
		T data[MaxSize];
		size_t currentSize = 0;

	public:
		FixedArray() : currentSize(0) {}

		void clear() {
			currentSize = 0;
		}

		bool add(const T& item) {
			if (currentSize >= MaxSize) return false;
			data[currentSize++] = item;
			return true;
		}

		bool add(T&& item) {
			if (currentSize >= MaxSize) return false;
			data[currentSize++] = std::move(item);
			return true;
		}

		size_t size() const {
			return currentSize;
		}

		size_t capacity() const {
			return MaxSize;
		}

		const T* begin() const {
			return data;
		}

		const T* end() const {
			return data + currentSize;
		}

		T* begin() {
			return data;
		}

		T* end() {
			return data + currentSize;
		}

		T& operator[](size_t index) {
			return data[index];
		}

		const T& operator[](size_t index) const {
			return data[index];
		}
	};

	struct Matrix4x4
	{
		float Matrix[4][4]{};
	};

	struct Vector2
	{
		float x, y;
	};

	struct Vector3
	{
		float x, y, z;

		__forceinline Vector3() : x(0), y(0), z(0) {}
		__forceinline Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

		Vector3 operator-(const Vector3& other) const {
			return Vector3(x - other.x, y - other.y, z - other.z);
		}

		__forceinline float Length() const {
			return sqrt(x * x + y * y + z * z);
		}

		__forceinline Vector3 operator+(const Vector3& other) const {
			__m128 a = _mm_set_ps(0, z, y, x);
			__m128 b = _mm_set_ps(0, other.z, other.y, other.x);
			__m128 result = _mm_add_ps(a, b);
			float temp[4];
			_mm_store_ps(temp, result);
			return Vector3(temp[0], temp[1], temp[2]);
		}

		Vector3 operator*(float scalar) const {
			return Vector3(x * scalar, y * scalar, z * scalar);
		}

		Vector3& operator+=(const Vector3& other) {
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		Vector3& operator-=(const Vector3& other) {
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}

		__forceinline float DistanceTo(const Vector3& other) const {
			return (*this - other).Length();
		}
	};

	struct Vector4
	{
		float x, y, z, w;
	};

	struct BonePosition {
		std::string name;
		int index;
	};

	struct BoundingBox {
		float x, y, w, h;
		bool valid;
	};

	struct Player {
		Vector3 Position;
		uintptr_t EntityPawn;
		int Health;
		int Armor;
		enums::ItemDefinitionIndex HoldingWeapon;
		int SteamId;
		enums::Team Team;
		bool Dormant;
		bool IsAlive;
		bool HasDefuser;
		bool HasHelmet;
		std::string Name;
	};

	struct Item {
		std::string Name;
		Vector3 Position;
		float Distance;
		bool IsValid;
		bool IsProjectile;
		int Clip;
		int MaxClipAmmo;

		Item() : Name(""), Position({ 0,0,0 }), Distance(0), IsValid(false) {}
	};

	struct C4
	{
		bool Activated;
		bool Planting;
		bool Exploded;
		bool Defused;
		Vector3 Position;
		bool BeingDefused;
		float TimeToExplode;
		float TimeToDefuse;
		float MaxTimeToExplode;
		float MaxTimeToDefuse;
		int Site; // 0 = A, 1 = B
	};
}

namespace vars {
	// pointer
	inline uintptr_t clientBase;
	inline uintptr_t engine2Base;
	// integer 
	inline DWORD pid;
	inline int boxStyle;
	inline int aimbotMode;
	inline int aimbotTarget;

	// boolean
	inline bool esp = true;
	inline bool showPlayerBox;
	inline bool showPlayerBone = true;
	inline bool showPlayerName = true;
	inline bool showTeammates;
	inline bool showHealthBar = true;
	inline bool showWeapon = true;
	inline bool showDistance = true;
	inline bool showArmor;

	inline bool aim;
	inline bool aimbot;
	inline bool showAimbotFov;
	inline bool rcs;
	inline bool showRcsDot = true;

	inline bool spectatorList = true;

	inline bool bombTimer = true;
	inline bool showItems = true;
	

	// float
	inline float textSize = 14.f;
	inline float maxFps = 45.f;
	inline float gameTime;
	inline float localSensitivity;


	inline float aimFov = 5.0f;
	inline float aimSmooth = 5.0f;

	// custom
	inline HANDLE driverHandle;
	inline structures::Player localPlayer;
	inline structures::Vector3 localViewAngel;
	inline structures::Matrix4x4 viewMatrix;
	inline structures::C4 C4;
}

namespace lists {

	inline structures::FixedArray<structures::Player, MAX_PLAYERS> Players;
	inline structures::FixedArray<structures::Item, MAX_ITEMS> Items;

	const std::vector<structures::BonePosition> BONES = {
			{ xorstr_("head"), 6 },
			{ xorstr_("neck_0"), 5 },
			{ xorstr_("spine_1"), 4 },
			{ xorstr_("spine_2"), 2 },
			{ xorstr_("pelvis"), 0 },
			{ xorstr_("arm_upper_L"), 8 },
			{ xorstr_("arm_lower_L"), 9 },
			{ xorstr_("hand_L"), 10 },
			{ xorstr_("arm_upper_R"), 13 },
			{ xorstr_("arm_lower_R"), 14 },
			{ xorstr_("hand_R"), 15 },
			{ xorstr_("leg_upper_L"), 22 },
			{ xorstr_("leg_lower_L"), 23 },
			{ xorstr_("ankle_L"), 24 },
			{ xorstr_("leg_upper_R"), 25 },
			{ xorstr_("leg_lower_R"), 26 },
			{ xorstr_("ankle_R"), 27 }
	};
	const std::vector<std::pair<std::string, std::string>> BONE_CONNECTIONS = {
		{xorstr_("head"), xorstr_("neck_0")},
		{xorstr_("neck_0"), xorstr_("spine_1")},
		{xorstr_("spine_1"), xorstr_("spine_2")},
		{xorstr_("spine_2"), xorstr_("pelvis")},
		{xorstr_("neck_0"), xorstr_("arm_upper_L")},
		{xorstr_("arm_upper_L"), xorstr_("arm_lower_L")},
		{xorstr_("arm_lower_L"), xorstr_("hand_L")},
		{xorstr_("neck_0"), xorstr_("arm_upper_R")},
		{xorstr_("arm_upper_R"), xorstr_("arm_lower_R")},
		{xorstr_("arm_lower_R"), xorstr_("hand_R")},
		{xorstr_("pelvis"), xorstr_("leg_upper_L")},
		{xorstr_("leg_upper_L"), xorstr_("leg_lower_L")},
		{xorstr_("leg_lower_L"), xorstr_("ankle_L")},
		{xorstr_("pelvis"), xorstr_("leg_upper_R")},
		{xorstr_("leg_upper_R"), xorstr_("leg_lower_R")},
		{xorstr_("leg_lower_R"), xorstr_("ankle_R")}
	};
}
