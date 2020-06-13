#include "PoshGiraffe.h"
#include "PoshMoveSet.h"
#include "PoshProjFuncs.h"

PoshGiraffe::PoshGiraffe(Vector2 _Position, COLORREF _Colour)
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
	Moves = new PoshMoveSet();
	Moves->InitMoves();
	Moves->InitThrows();
	Moves->InitTilts();
	Moves->InitSmashes();
	Moves->InitAerials();
	Moves->InitSpecials();

	//Misc
	Stocks = MAX_STOCKS;
	Knockback = 0;
	Mass = 80;
	CommandGrabPointer = 0;
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
	delete Moves;
}

void PoshGiraffe::UniqueChanges(std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes, const int num_giraffes, const int i, const int inputs, const int frameNumber, Stage& stage)
{
	//Grab
	if (State & STATE_WEAK && State & STATE_HEAVY && AnimFrame == 7) {
		Collider grabCol = { {Position.x + 1.885000f * Facing.x, Position.y - 0.650000f}, 0.855862f };
		for (int j = 0; j < num_giraffes; ++j) {
			if (j != i && giraffes[j]->Stocks > 0) {
				if (giraffes[j]->GrabHit(grabCol, Facing, frameNumber)) {
					State &= ~(STATE_WEAK | STATE_HEAVY | STATE_RUNNING);
					State |= STATE_GRABBING;
					TechDelay = frameNumber + 30;
					break;
				}
			}
		}
	}
	
	if (State & STATE_HEAVY) {
		if (!(State & (STATE_WEAK | STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD)) && AnimFrame == 5) {
			ChangeHat();
		}

		else if (State & STATE_JUMPING && State & STATE_FORWARD && AnimFrame == 7) {
			switch (Hat) {
			case 1:
				Projectiles.Append(Projectile(Position + Facing * (*Moves->GetSkelPoints(AttackNum, AnimFrame))[31], { 0.4f * Facing.x, 0.0f }, 0.3f, { 0.3f * Facing.x, -0.7f }, 0.3f, 0.1f, 0.4f, false, LastAttackID, frameNumber + 100, PoshProjFuncs::StandardOnHit, PoshProjFuncs::StandardUpdate, PoshProjFuncs::SombreroDraw, GiraffePen, nullptr));
				break;
			case 2:
				Projectiles.Append(Projectile(Position + Facing * (*Moves->GetSkelPoints(AttackNum, AnimFrame))[31], { Facing.x, 0.0f }, 0.3f, { 0.4f * Facing.x, 0.1f }, 0.3f, 0.1f, 0.4f, false, LastAttackID, frameNumber + 100, PoshProjFuncs::StandardOnHit, PoshProjFuncs::StandardUpdate, PoshProjFuncs::RobinDraw, GiraffePen, nullptr));
				break;
			case 3:
				Projectiles.Append(Projectile(Position + Facing * (*Moves->GetSkelPoints(AttackNum, AnimFrame))[31], { 0.25f * Facing.x, 0.0f }, 0.3f, { Facing.x, 0.0f }, 0.6f, 0.1f, 0.4f, false, LastAttackID, frameNumber + 100, PoshProjFuncs::StandardOnHit, PoshProjFuncs::StandardUpdate, PoshProjFuncs::CrownDraw, GiraffePen, nullptr));
				break;
			default:
				Projectiles.Append(Projectile(Position + Facing * (*Moves->GetSkelPoints(AttackNum, AnimFrame))[31], { 0.5f * Facing.x, 0.0f }, 0.3f, { Facing.x, -0.2 }, 0.4f, 0.1f, 0.5f, false, LastAttackID, frameNumber + 100, PoshProjFuncs::StandardOnHit, PoshProjFuncs::StandardUpdate, PoshProjFuncs::TopHatDraw, GiraffePen, nullptr));
			}
			ChangeHat();
		}

		else if (State & STATE_JUMPING && State & STATE_UP) {
			if (AnimFrame == 0) {
				Velocity.y = 0;
			}
			else if (AnimFrame >= 7 && AnimFrame <= 20) {
				Velocity.y -= 0.075f;
			}
			else if (AnimFrame == 21) {
				Velocity.y *= 0.8f;
			}
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
			if (j != i && giraffes[j]->Stocks > 0) {
				if (giraffes[j]->ProjectileHit(Projectiles[p])) {
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
			if (j != i && giraffes[j]->Stocks > 0) {
				for (int h = 0; h < numHitboxes; ++h) {
					HitCollider hitbox = (*Hitboxes)[h];
					hitbox.Damage *= Multiplier;
					if (giraffes[j]->AddHit(hitbox, LastAttackID, Facing, Position)) {
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

void PoshGiraffe::RecieveHits(Stage& stage, const int frameNumber)
{
	if (incomingGrab) {
		State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_SHIELDING | STATE_DROPSHIELD | STATE_SHIELDSTUN | STATE_JUMPSQUAT | STATE_JUMPLAND | STATE_HITSTUN | STATE_FASTFALL | STATE_DOUBLEJUMPWAIT | STATE_WAVEDASH | STATE_RUNNING | STATE_SHORTHOP | STATE_CROUCH | STATE_TECHATTEMPT | STATE_TECHLAG | STATE_TECHING | STATE_KNOCKDOWN | STATE_KNOCKDOWNLAG | STATE_ROLLING | STATE_GETUPATTACK | STATE_GRABBING | STATE_THROW);
		State |= STATE_GRABBED;
		SoundMoveState &= ~(SOUND_SHIELD | SOUND_RUN);
		incomingGrab = false;

	}
	if (State & STATE_SHIELDING) {
		for (int j = 0; j < numIncoming; ++j) {
			State |= STATE_SHIELDSTUN;
			AttackDelay = (int)max(AttackDelay, frameNumber + IncomingHits[j].Damage * 50);
		}
	}
	else {
		for (int j = 0; j < numIncoming; ++j) {
			Knockback += IncomingHits[j].Damage * Multiplier;
			float KnockbackApplied;
			if (IncomingHits[j].Fixed) {
				KnockbackApplied = IncomingHits[j].Knockback;
			}
			else {
				KnockbackApplied = ((((Knockback / 10 + (Knockback * IncomingHits[j].Damage * Multiplier / 20)) * (200 / (Mass + 100)) * 1.4f + 0.18f) * IncomingHits[j].Scale) + IncomingHits[j].Knockback);
			}
			Velocity += KnockbackApplied * IncomingHits[j].Force * 0.35f;
			if (State & STATE_LEDGEHOG) {
				stage.Ledges[LedgeID].Hogged = false;
				State &= ~STATE_LEDGEHOG;
			}
			State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_JUMPSQUAT | STATE_JUMPLAND | STATE_SHORTHOP | STATE_KNOCKDOWN | STATE_KNOCKDOWNLAG | STATE_GETUPATTACK | STATE_GRABBING | STATE_GRABBED | STATE_THROW);
			State |= STATE_HITSTUN;
			SoundMoveState |= SOUND_HITSTUN;
			SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_HITSTUN] = 100000000;
			AttackDelay = (int)max(AttackDelay, frameNumber + min(KnockbackApplied * 40, 100));
		}
	}
}

void PoshGiraffe::Landing(Stage& stage, const int frameNumber, std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes)
{
	if ((State & STATE_HEAVY) && (State & STATE_DOWN)) {
		//State &= ~STATE_JUMPING;
		Velocity.x = 0;
		HasAirDash = true;
		HasDoubleJump = false;
	}
	else {
		SoundMoveState |= SOUND_JUMPLAND;
		SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_JUMPLAND] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_JUMPLAND);
		if (State & STATE_HITSTUN && !(State & STATE_TECHATTEMPT)) {
			if (Velocity.x > 0) {
				Facing.x = -1;
			}
			else {
				Facing.x = 1;
			}
			Velocity = { 0,0 };
			State |= STATE_KNOCKDOWNLAG;
			State &= ~STATE_HITSTUN;
			AnimFrame = 0;
			TechDelay = frameNumber + 30;
			SoundMoveState |= SOUND_KNOCKDOWN;
			SoundMoveState &= ~SOUND_HITSTUN;
			SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_KNOCKDOWN] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_KNOCKDOWN);
		}
		else if (State & STATE_GRABBING) {
			State &= ~STATE_GRABBING;
			giraffes[CommandGrabPointer]->State &= ~STATE_GRABBED;
		}
		else {
			if (State & (STATE_WEAK | STATE_HEAVY)) {
				State |= STATE_HITSTUN;
				AttackDelay = frameNumber + Moves->GetLandingLag(max(0, (AttackNum - 25)));
				SoundMoveState |= SOUND_HITSTUN;
				SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_HITSTUN] = AttackDelay;
			}
			else if ((State & STATE_TECHATTEMPT) && (State & STATE_HITSTUN)) {
				State &= ~STATE_TECHATTEMPT;
				State |= STATE_TECHING | STATE_INTANGIBLE;
				TechDelay = frameNumber + 20;
				SoundMoveState &= ~SOUND_HITSTUN;
				SoundMoveState |= SOUND_TECH;
				SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_TECH] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_TECH);
			}

			State |= STATE_JUMPLAND;
			HasAirDash = true;
			HasDoubleJump = false;
			JumpDelay = frameNumber + MaxJumpDelay / 2;
			AnimFrame = 0;
		}
		SoundAttackState &= ~(SOUND_DOWNB | SOUND_UPB);
		State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_JUMPING | STATE_FASTFALL | STATE_TECHLAG | STATE_THROW | STATE_GRABBED | STATE_GRABBING);
	}
}

void PoshGiraffe::ParseWalk(const int inputs, const int frameNumber, Stage& stage)
{
	if (State & STATE_HEAVY && State & STATE_DOWN) {
		return;
	}
	if (State & STATE_LEDGEHOG) {
		if (inputs & INPUT_DOWN) {
			State &= ~STATE_LEDGEHOG;
			stage.Ledges[LedgeID].Hogged = false;
			State |= STATE_JUMPING | STATE_DOUBLEJUMPWAIT;
			JumpDelay = frameNumber + MaxJumpDelay * 2;
			SoundMoveState &= ~SOUND_HITSTUN;
		}
	}
	else if (State & STATE_GRABBING) {
		if ((inputs & INPUT_RIGHT && Facing.x == 1) || (inputs & INPUT_LEFT && Facing.x == -1)) {
			State &= ~STATE_GRABBING;
			State |= STATE_FORWARD | STATE_THROW;
			AttackNum = 13;
			AttackDelay = frameNumber + Moves->GetMoveLength(AttackNum);
			AnimFrame = 0;
			LastAttackID++;
			SoundAttackState |= SOUND_FTHROW;
			SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_FTHROW] = frameNumber + Moves->GetAttackSoundLength(XACT_WAVEBANK_ATTACKBANK_FTHROW);
		}
		else if (inputs & INPUT_UP) {
			State &= ~STATE_GRABBING;
			State |= STATE_UP | STATE_THROW;
			AttackNum = 14;
			AttackDelay = frameNumber + Moves->GetMoveLength(AttackNum);
			AnimFrame = 0;
			LastAttackID++;
			SoundAttackState |= SOUND_UPTHROW;
			SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_UPTHROW] = frameNumber + Moves->GetAttackSoundLength(XACT_WAVEBANK_ATTACKBANK_UPTHROW);
		}
		else if (inputs & INPUT_DOWN) {
			State &= ~STATE_GRABBING;
			State |= STATE_DOWN | STATE_THROW;
			AttackNum = 15;
			AttackDelay = frameNumber + Moves->GetMoveLength(AttackNum);
			AnimFrame = 0;
			LastAttackID++;
			SoundAttackState |= SOUND_DOWNTHROW;
			SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_DOWNTHROW] = frameNumber + Moves->GetAttackSoundLength(XACT_WAVEBANK_ATTACKBANK_DOWNTHROW);
		}
		else if ((inputs & INPUT_LEFT && Facing.x == 1) || (inputs & INPUT_RIGHT && Facing.x == -1)) {
			State &= ~STATE_GRABBING;
			State |= STATE_BACK | STATE_THROW;
			AttackNum = 16;
			AttackDelay = frameNumber + Moves->GetMoveLength(AttackNum);
			AnimFrame = 0;
			LastAttackID++;
			SoundAttackState |= SOUND_BACKTHROW;
			SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_BACKTHROW] = frameNumber + Moves->GetAttackSoundLength(XACT_WAVEBANK_ATTACKBANK_BACKTHROW);
		}
	}
	else if (State & STATE_HITSTUN) {
		if (State & STATE_JUMPING) {
			if (inputs & INPUT_LEFT && Velocity.x > -MaxAirSpeed.x) {
				Velocity.x -= AirAccel / 10.0f;
			}
			else if (inputs & INPUT_RIGHT && Velocity.x < MaxAirSpeed.x) {
				Velocity.x += AirAccel / 10.0f;
			}
			if (inputs & INPUT_UP && Velocity.y > -MaxAirSpeed.y) {
				Velocity.y -= AirAccel / 10.0f;
			}
			else if (inputs & INPUT_DOWN && Velocity.y < MaxAirSpeed.y) {
				Velocity.y += AirAccel / 10.0f;
			}
		}
	}
	else if (State & STATE_JUMPING) {
		if (inputs & INPUT_LEFT && Velocity.x > -MaxAirSpeed.x) {
			Velocity.x -= AirAccel;
		}
		else if (inputs & INPUT_RIGHT && Velocity.x < MaxAirSpeed.x) {
			Velocity.x += AirAccel;
		}
		if (inputs & INPUT_DOWN && !(inputs & (INPUT_WEAK | INPUT_HEAVY)) && !(State & STATE_FASTFALL)) {
			State |= STATE_FASTFALL;
			SoundMoveState |= SOUND_FASTFALL;
			SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_FASTFALL] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_FASTFALL);
		}
	}
	else if (State & (STATE_TECHING | STATE_SHIELDING | STATE_KNOCKDOWN)) {
		if (inputs & INPUT_LEFT) {
			State &= ~(STATE_TECHING | STATE_SHIELDING | STATE_KNOCKDOWN | STATE_CROUCH);
			State |= STATE_ROLLING | STATE_INTANGIBLE;
			AttackDelay = frameNumber + 20;
			AnimFrame = 0;
			Facing = { -1, 1 };
			SoundMoveState |= SOUND_ROLL;
			SoundMoveState &= ~SOUND_SHIELD;
			SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_ROLL] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_ROLL);
		}
		else if (inputs & INPUT_RIGHT) {
			State &= ~(STATE_TECHING | STATE_SHIELDING | STATE_KNOCKDOWN | STATE_CROUCH);
			State |= STATE_ROLLING | STATE_INTANGIBLE;
			AttackDelay = frameNumber + 20;
			AnimFrame = 0;
			Facing = { 1, 1 };
			SoundMoveState |= SOUND_ROLL;
			SoundMoveState &= ~SOUND_SHIELD;
			SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_ROLL] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_ROLL);
		}
	}
	else if (!(State & (STATE_WEAK | STATE_HEAVY))) {
		if (inputs & INPUT_LEFT && Velocity.x > -MaxGroundSpeed) {
			Velocity.x -= RunAccel;
			State &= ~STATE_CROUCH;
			State |= STATE_RUNNING;
			Facing = { -1, 1 };
			SoundMoveState |= SOUND_RUN;
			SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_RUN] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_RUN);
		}
		else if (inputs & INPUT_RIGHT && Velocity.x < MaxGroundSpeed) {
			Velocity.x += RunAccel;
			State &= ~STATE_CROUCH;
			State |= STATE_RUNNING;
			Facing = { 1, 1 };
			SoundMoveState |= SOUND_RUN;
			SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_RUN] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_RUN);
		}
		else if (!(inputs & (INPUT_LEFT | INPUT_RIGHT))) {
			State &= ~STATE_RUNNING;
			SoundMoveState &= ~SOUND_RUN;
		}

		if (inputs & INPUT_DOWN && !(inputs & (INPUT_WEAK | INPUT_HEAVY)) && !(State & (STATE_CROUCH | STATE_ROLLING))) {
			State |= STATE_CROUCH;
			SoundMoveState |= SOUND_CROUCH;
			SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_CROUCH] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_CROUCH);
		}
		else if (!(inputs & INPUT_DOWN) && State & STATE_CROUCH) {
			State &= ~STATE_CROUCH;
		}
	}
}

void PoshGiraffe::ChangeHat()
{
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
