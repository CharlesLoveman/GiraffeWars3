#include "PoshGiraffe.h"

PoshGiraffe::PoshGiraffe(Vector2 _Position, MoveSet* _Moves, COLORREF _Colour)
{
	//Movement
	Position = _Position;
	Velocity = Vector2(0, 0);
	MaxGroundSpeed = 0.4f;
	MaxAirSpeed = Vector2(0.3f, 0.7f);
	RunAccel = 0.08f;
	AirAccel = 0.028f;
	Gravity = 0.015f;
	Facing = { 1.0f, 1.0f };

	//Jumping
	JumpSpeed = 0.5f;
	HasDoubleJump = false;
	DashSpeed = 0.4f;
	HasAirDash = true;

	//Collision
	Fullbody = { Vector2(0.0f,0.0f), 2.5f };
	StageCollider = { Vector2(0.0f,0.7f), 1.0f };
	Hitboxes = nullptr;
	Hurtboxes = nullptr;
	numHitboxes = 0;
	numIncoming = 0;
	PrevHitQueue = ArrayQueue<int>();
	LastAttackID = 0;
	Projectiles = ArrayList<Projectile>();
	incomingGrab = false;

	//State
	State = 0;
	SoundMoveState = 0;
	SoundAttackState = 0;
	JumpDelay = 0;
	MaxJumpDelay = 4;
	AttackDelay = 0;
	AttackNum = 0;
	MaxShieldDelay = 5;
	TechDelay = 0;
	Moves = _Moves;

	//Misc
	Stocks = 3;
	Knockback = 0;
	Mass = 80;
	CommandGrabPointer = 0;
	/*Hat = 0;
	Multiplier = 1.0f;*/
	Hat = 0;
	Multiplier = 1.0f;

	//Animation
	AnimFrame = 0;
	GiraffePen = CreatePen(PS_SOLID, 1, _Colour);
	IntangiblePen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	ShieldBrush = CreateHatchBrush(HS_BDIAGONAL, RGB(0, 255, 127));
	MonocleBrush = CreateSolidBrush(0);
	TieBrush = CreateHatchBrush(HS_BDIAGONAL, _Colour);
}

PoshGiraffe::~PoshGiraffe()
{
}

void PoshGiraffe::UniqueChanges(std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes, const int num_giraffes, const int i, const int inputs, const int frameNumber, Stage& stage)
{
	//Grab
	if (State & STATE_WEAK && State & STATE_HEAVY && AnimFrame == 7) {
		Collider grabCol = { {Position.x + 1.885000f * Facing.x, Position.y - 0.650000f}, 0.855862f };
		for (int j = 0; j < num_giraffes; ++j) {
			if (j != i) {
				if (giraffes[j]->GrabHit(grabCol, Facing, frameNumber)) {
					State &= ~(STATE_WEAK | STATE_HEAVY | STATE_RUNNING);
					State |= STATE_GRABBING;
					TechDelay = frameNumber + 30;
					break;
				}
			}
		}
	}
	
	if (State & STATE_HEAVY && !(State & (STATE_WEAK | STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD)) && AnimFrame == 5) {
		Hat = (Hat + 1) % 4;
		switch (Hat) {
		case 1:
			MaxGroundSpeed = 0.3f;
			MaxAirSpeed = Vector2(0.3f, 0.7f);
			RunAccel = 0.06f;
			AirAccel = 0.024f;
			Gravity = 0.01f;
			Multiplier = 0.6f;
			break;
		case 2:
			MaxGroundSpeed = 0.6f;
			MaxAirSpeed = Vector2(0.5f, 0.7f);
			RunAccel = 0.1f;
			AirAccel = 0.032f;
			Gravity = 0.03f;
			Multiplier = 0.8f;
			break;
		case 3:
			MaxGroundSpeed = 0.3f;
			MaxAirSpeed = Vector2(0.2f, 0.5f);
			RunAccel = 0.05f;
			AirAccel = 0.02f;
			Gravity = 0.02f;
			Multiplier = 1.2f;
			break;
		default:
			MaxGroundSpeed = 0.4f;
			MaxAirSpeed = Vector2(0.3f, 0.7f);
			RunAccel = 0.08f;
			AirAccel = 0.028f;
			Gravity = 0.015f;
			Multiplier = 1.0f;
		}
	}
}

void PoshGiraffe::Draw(HDC hdc, Vector2 Scale, int frameNumber)
{
	for (int i = 0; i < Projectiles.Size(); ++i) {
		Projectiles[i].Draw(Projectiles[i], *this, hdc, Scale, frameNumber);
	}

	int CurrentAnim = 0;
	int CurrentFrame = 0;

	if (State & (STATE_WEAK | STATE_HEAVY | STATE_THROW)) {
		CurrentAnim = AttackNum;
		CurrentFrame = AnimFrame;
	}
	else {
		if (State & STATE_HITSTUN) {
			CurrentAnim = 6;
			CurrentFrame = AnimFrame % 9;
		}
		else if (State & STATE_SHIELDSTUN) {
			SelectObject(hdc, ShieldBrush);
			DrawHitbox(hdc, Scale, { 0,0 }, 2.5f);
			CurrentAnim = 6;
			CurrentFrame = AnimFrame % 9;
		}
		else if (State & STATE_SHIELDING) {
			SelectObject(hdc, ShieldBrush);
			DrawHitbox(hdc, Scale, { 0,0 }, 2.5f);
			CurrentAnim = 0;
			CurrentFrame = 0;
		}
		else if (State & STATE_RUNNING) {
			CurrentAnim = 1;
			CurrentFrame = AnimFrame % 21;
		}
		else if (State & STATE_JUMPING) {
			CurrentAnim = 2;
			CurrentFrame = 0;
		}
		else if (State & STATE_JUMPSQUAT) {
			CurrentAnim = 3;
			CurrentFrame = AnimFrame;
		}
		else if (State & STATE_JUMPLAND) {
			CurrentAnim = 4;
			CurrentFrame = AnimFrame;
		}
		else if (State & STATE_KNOCKDOWNLAG) {
			CurrentAnim = 5;
			CurrentFrame = AnimFrame;
		}
		else if (State & STATE_KNOCKDOWN) {
			CurrentAnim = 5;
			CurrentFrame = 30;
		}
		else if (State & STATE_LEDGEHOG) {
			CurrentAnim = 6;
			CurrentFrame = AnimFrame % 9;
		}
		else if (State & (STATE_WAVEDASH | STATE_CROUCH)) {
			CurrentAnim = 7;
			CurrentFrame = 0;
		}
		else if (State & STATE_ROLLING) {
			CurrentAnim = 8;
			CurrentFrame = AnimFrame;
		}
		else if (State & STATE_GRABBED) {
			CurrentAnim = 6;
			CurrentFrame = AnimFrame % 9;
		}
		else if (State & STATE_GRABBING) {
			CurrentAnim = 12;
			CurrentFrame = 7;
		}
	}
	DrawSelf(hdc, Scale, CurrentFrame, CurrentAnim);
}

int PoshGiraffe::Size()
{
	return sizeof(*this);
}

void PoshGiraffe::DrawSelf(HDC hdc, Vector2 Scale, int CurrentFrame, int CurrentAnim)
{
	if (State & STATE_INTANGIBLE) {
		SelectObject(hdc, IntangiblePen);
	}
	else {
		SelectObject(hdc, GiraffePen);
	}
	std::vector<POINT> points;
	std::vector<Vector2> vPoints = (*Moves->GetSkelPoints(CurrentAnim, CurrentFrame));
	
	switch (Hat)
	{
	case 1:
		DrawSombrero(hdc, Scale, vPoints[33], vPoints[32]);
		break;
	case 2:
		DrawRobin(hdc, Scale, vPoints[33], vPoints[32]);
		break;
	case 3:
		DrawCrown(hdc, Scale, vPoints[33], vPoints[32]);
		break;
	default:
		DrawTopHat(hdc, Scale, vPoints[33], vPoints[32]);
	}

	for (int i = 0; i < vPoints.size(); ++i) {
		points.push_back(Giraffe::VecToPoint(Position + Facing * vPoints[i], Scale));
	}

	Polyline(hdc, &points[0], 27);
	PolyBezier(hdc, &points[26], 4);
	Polyline(hdc, &points[29], 5);
	PolyBezier(hdc, &points[33], 4);
	Polyline(hdc, &points[36], 2);


	if (vPoints.size() > 38) {
		DrawTie(hdc, Scale, vPoints[39], vPoints[32] - vPoints[30]);
	}
}

void PoshGiraffe::DrawHitbox(HDC hdc, Vector2 Scale, Vector2 Pos, float Rad)
{
	SelectObject(hdc, GiraffePen);
	Ellipse(hdc, Scale.x * (Position.x + Facing.x * Pos.x - Rad), Scale.y * (Position.y + Facing.y * Pos.y - Rad), Scale.x * (Position.x + Facing.x * Pos.x + Rad), Scale.y * (Position.y + Facing.y * Pos.y + Rad));
}

void PoshGiraffe::DrawTopHat(HDC hdc, Vector2 Scale, Vector2 Head1, Vector2 Head2)
{
	Vector2 perp = Head1 - Head2;
	perp.Normalize();
	Vector2 dir = { -perp.y, perp.x };
	Vector2 pos = 0.5f * (Head1 + Head2);

	POINT points[6];
	points[0] = VecToPoint(Position + Facing * (pos + perp * 0.5f), Scale);
	points[1] = VecToPoint(Position + Facing * (pos + perp * 0.2f), Scale);
	points[2] = VecToPoint(Position + Facing * (pos + perp * 0.2f + dir * 0.75f), Scale);
	points[3] = VecToPoint(Position + Facing * (pos + perp * -0.2f + dir * 0.75f), Scale);
	points[4] = VecToPoint(Position + Facing * (pos + perp * -0.2f), Scale);
	points[5] = VecToPoint(Position + Facing * (pos + perp * -0.5f), Scale);

	Polyline(hdc, points, 6);
}

void PoshGiraffe::DrawSombrero(HDC hdc, Vector2 Scale, Vector2 Head1, Vector2 Head2)
{
	Vector2 perp = Head1 - Head2;
	perp.Normalize();
	Vector2 dir = { -perp.y, perp.x };
	Vector2 pos = 0.5f * (Head1 + Head2);

	POINT points[9];
	points[0] = VecToPoint(Position + Facing * (pos + perp * 0.7f + dir * 0.3f), Scale);
	points[1] = VecToPoint(Position + Facing * (pos + perp * 0.8f), Scale);
	points[2] = VecToPoint(Position + Facing * (pos + perp * 0.2f), Scale);
	points[3] = VecToPoint(Position + Facing * (pos + perp * 0.1f + dir * 0.4f), Scale);
	points[4] = VecToPoint(Position + Facing * (pos + dir * 0.5f), Scale);
	points[5] = VecToPoint(Position + Facing * (pos + perp * -0.1f + dir * 0.4f), Scale);
	points[6] = VecToPoint(Position + Facing * (pos + perp * -0.2f), Scale);
	points[7] = VecToPoint(Position + Facing * (pos + perp * -0.8f), Scale);
	points[8] = VecToPoint(Position + Facing * (pos + perp * -0.7f + dir * 0.3f), Scale);

	Polyline(hdc, points, 9);
}

void PoshGiraffe::DrawRobin(HDC hdc, Vector2 Scale, Vector2 Head1, Vector2 Head2)
{
	Vector2 perp = Head1 - Head2;
	perp.Normalize();
	Vector2 dir = { -perp.y, perp.x };
	Vector2 pos = 0.5f * (Head1 + Head2);

	POINT points[6];
	points[0] = VecToPoint(Position + Facing * (pos + perp * 0.2f), Scale);
	points[1] = VecToPoint(Position + Facing * (pos + perp * 0.5f), Scale);
	points[2] = VecToPoint(Position + Facing * (pos + perp * 0.45f + dir * 0.225f), Scale);
	points[3] = VecToPoint(Position + Facing * (pos + perp * -0.25f + dir * 0.225f), Scale);
	points[4] = VecToPoint(Position + Facing * (pos + perp * -0.6f), Scale);
	points[5] = VecToPoint(Position + Facing * (pos + perp * -0.2f), Scale);
	Polyline(hdc, points, 6);

	points[0] = VecToPoint(Position + Facing * (pos + perp * 0.4f + dir * 0.225f), Scale);
	points[1] = VecToPoint(Position + Facing * (pos + perp * 0.2f + dir * 0.6f), Scale);
	points[2] = VecToPoint(Position + Facing * (pos + perp * -0.18f + dir * 0.225f), Scale);
	Polyline(hdc, points, 3);
}

void PoshGiraffe::DrawCrown(HDC hdc, Vector2 Scale, Vector2 Head1, Vector2 Head2)
{
	Vector2 perp = Head1 - Head2;
	perp.Normalize();
	Vector2 dir = { -perp.y, perp.x };
	Vector2 pos = 0.5f * (Head1 + Head2);

	POINT points[9];
	points[0] = VecToPoint(Position + Facing * (pos + perp * 0.2f), Scale);
	points[1] = VecToPoint(Position + Facing * (pos + perp * 0.45f + dir * 0.5f), Scale);
	points[2] = VecToPoint(Position + Facing * (pos + perp * 0.3f + dir * 0.3f), Scale);
	points[3] = VecToPoint(Position + Facing * (pos + perp * 0.15f + dir * 0.6f), Scale);
	points[4] = VecToPoint(Position + Facing * (pos + dir * 0.35f), Scale);
	points[5] = VecToPoint(Position + Facing * (pos + perp * -0.15f + dir * 0.6f), Scale);
	points[6] = VecToPoint(Position + Facing * (pos + perp * -0.3f + dir * 0.3f), Scale);
	points[7] = VecToPoint(Position + Facing * (pos + perp * -0.45f + dir * 0.5f), Scale);
	points[8] = VecToPoint(Position + Facing * (pos + perp * -0.2f), Scale);

	Polyline(hdc, points, 9);
}

void PoshGiraffe::DrawTie(HDC hdc, Vector2 Scale, Vector2 Pos, Vector2 Dir)
{
	//SelectObject(hdc, TieBrush);
	Dir.Normalize();
	Vector2 Perp = { -Dir.y, Dir.x };
	Pos -= Dir * 0.3f;

	POINT points[8];
	points[0] = VecToPoint(Position + Facing * (Pos), Scale);
	points[1] = VecToPoint(Position + Facing * (Pos + Dir * 0.2f + Perp * 0.2f), Scale);
	points[2] = VecToPoint(Position + Facing * (Pos + Dir * 0.2f + Perp * -0.2f), Scale);
	points[3] = VecToPoint(Position + Facing * (Pos), Scale);
	points[4] = VecToPoint(Position + Facing * (Pos + Dir * -0.6f + Perp * 0.2f), Scale);
	points[5] = VecToPoint(Position + Facing * (Pos + Dir * -0.8f), Scale);
	points[6] = VecToPoint(Position + Facing * (Pos + Dir * -0.6f + Perp * -0.2f), Scale);
	points[7] = VecToPoint(Position + Facing * (Pos), Scale);

	//Polygon(hdc, points, 8);
	Polyline(hdc, points, 8);
}

void PoshGiraffe::GiveHits(std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes, const int num_giraffes, const int frameNumber, const int i)
{
	//Apply hits to other giraffes
	for (int p = Projectiles.Size() - 1; p >= 0; --p) {
		for (int j = 0; j < num_giraffes; ++j) {
			if (j != i) {
				if ((*giraffes[j]).ProjectileHit(Projectiles[p])) {
					Projectiles[p].OnHit(Projectiles[p], *this, giraffes[j], frameNumber);
					Projectiles.Remove(p);
					SoundAttackState |= SOUND_WEAK;
					SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_WEAK] = frameNumber + Moves->GetAttackSoundLength(XACT_WAVEBANK_ATTACKBANK_WEAK);
					break;
				}
			}
		}
	}


	if (!(Hitboxes == nullptr)) {
		for (int j = 0; j < num_giraffes; ++j) {
			if (j != i) {
				for (int h = 0; h < numHitboxes; ++h) {
					HitCollider hitbox = (*Hitboxes)[h];
					hitbox.Damage *= Multiplier;
					if ((*giraffes[j]).AddHit(hitbox, LastAttackID, Facing, Position)) {
						if (hitbox.Damage > 1) {
							SoundAttackState |= SOUND_HEAVY;
							SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_HEAVY] = frameNumber + Moves->GetAttackSoundLength(XACT_WAVEBANK_ATTACKBANK_HEAVY);
						}
						else if (hitbox.Damage > 0.5f) {
							SoundAttackState |= SOUND_MEDIUM;
							SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_MEDIUM] = frameNumber + Moves->GetAttackSoundLength(XACT_WAVEBANK_ATTACKBANK_MEDIUM);
						}
						else {
							SoundAttackState |= SOUND_WEAK;
							SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_WEAK] = frameNumber + Moves->GetAttackSoundLength(XACT_WAVEBANK_ATTACKBANK_WEAK);
						}
					}
				}
			}
		}
	}
}
