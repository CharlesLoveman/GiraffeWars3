#include "CoolGiraffe.h"
#include "CoolProjFuncs.h"

CoolGiraffe::CoolGiraffe(Vector2 _Position, COLORREF _Colour)
{
	//Movement
	Position = _Position;
	Velocity = Vector2(0, 0);
	MaxGroundSpeed = 0.4f;
	MaxAirSpeed = Vector2(0.3f, 0.7f);
	RunAccel = 0.1f;
	AirAccel = 0.03f;
	Gravity = 0.025f;
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
	Moves = new CoolMoveSet();
	Moves->InitMoves();
	Moves->InitThrows();
	Moves->InitTilts();
	Moves->InitSmashes();
	Moves->InitAerials();
	Moves->InitSpecials();

	//Misc
	Stocks = 3;
	Knockback = 0;
	Mass = 80;
	CommandGrabPointer = 0;

	//Animation
	AnimFrame = 0;
	GiraffePen = CreatePen(PS_SOLID, 1, _Colour);
	IntangiblePen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	ShieldBrush = CreateHatchBrush(HS_BDIAGONAL, RGB(0, 255, 127));
	FirePen = CreatePen(PS_SOLID, 1, RGB(230, 128, 40));
}

CoolGiraffe::~CoolGiraffe()
{
	delete Moves;
}

void CoolGiraffe::UniqueChanges(std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes, const int num_giraffes, const int i, const int inputs, const int frameNumber, Stage& stage)
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


	//Character Move-Specific Updates
	if ((State & STATE_JUMPING) && (State & STATE_HEAVY) && (State & STATE_FORWARD) && AnimFrame == 13) {
		Projectiles.Append(Projectile(Position + Vector2(Facing.x, 0), { 0.1f * Facing.x, 0 }, 1.0f, {Facing.x, 0}, 0.5f, 0.0f, 0.3f, false, LastAttackID, frameNumber + 1000, CoolProjFuncs::StandardOnHit, CoolProjFuncs::StandardUpdate, CoolProjFuncs::FireballDraw, FirePen, nullptr));
	}


	if ((State & STATE_HEAVY) && (State & STATE_UP)) {
		if (State & STATE_JUMPING && AnimFrame >= 13 && AnimFrame <= 23) {
			if (AnimFrame == 14) {
				Velocity.y = 0;
			}
			Velocity += Vector2(0, -0.2f);
		}
		//Flip during dp
		else if (AnimFrame == 24) {
			Facing.x *= -1;
		}
	}
	//Reverse direction in bair
	else if ((State & STATE_JUMPING) && (State & STATE_WEAK) && (State & STATE_BACK)) {
		if (AnimFrame == 4 || AnimFrame == 20) {
			Facing.x *= -1;
		}
		else if (AnimFrame == 8) {
			LastAttackID++;
		}
	}
	//Reverse direction in dtilt
	else if (!(State & STATE_JUMPING) && (State & STATE_WEAK) && (State & STATE_DOWN) && (AnimFrame == 7 || AnimFrame == 0)) {
		Facing.x *= -1;
	}
}

void CoolGiraffe::Draw(HDC hdc, Vector2 Scale, int frameNumber)
{
	for (int i = 0; i < Projectiles.Size(); ++i) {
		Projectiles[i].Draw(Projectiles[i], *this, hdc, Scale, frameNumber);
	}

	int CurrentAnim = 0;
	int CurrentFrame = 0;
	

	if (State & STATE_HEAVY) {
		DrawSelf(hdc, Scale, AnimFrame, AttackNum);
		if (State & STATE_FORWARD && AnimFrame >= 15 && AnimFrame <= 25) {
			DrawSmallFlame(hdc, Scale, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[24] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[27]) * 0.5f, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[25] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[26]) * 0.5f);
		}
		else if (State & STATE_UP && AnimFrame >= 19 && AnimFrame <= 30) {
			DrawSmallFlame(hdc, Scale, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[24] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[27]) * 0.5f, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[25] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[26]) * 0.5f);
		}
		else if (!(State & (STATE_UP | STATE_FORWARD | STATE_DOWN)) && AnimFrame >= 50 && AnimFrame <= 70) {
			DrawLargeFlame(hdc, Scale, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[32] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[36]) * 0.5f, (*Moves->GetSkelPoints(AttackNum, AnimFrame))[34]);
		}
		return;
	}

	if (State & (STATE_WEAK | STATE_HEAVY | STATE_THROW)) {
		CurrentAnim = AttackNum;
		CurrentFrame = AnimFrame;
	}
	else {
		if (State & STATE_HITSTUN) {
			CurrentAnim = 6;
			CurrentFrame = AnimFrame % 16;
		}
		else if (State & STATE_SHIELDSTUN) {
			SelectObject(hdc, ShieldBrush);
			DrawHitbox(hdc, Scale, { 0,0 }, 2.5f);
			CurrentAnim = 6;
			CurrentFrame = AnimFrame % 16;
		}
		else if (State & STATE_SHIELDING) {
			SelectObject(hdc, ShieldBrush);
			DrawHitbox(hdc, Scale, { 0,0 }, 2.5f);
			CurrentAnim = 0;
			CurrentFrame = 0;
		}
		else if (State & STATE_RUNNING) {
			CurrentAnim = 1;
			CurrentFrame = AnimFrame % 36;
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
			CurrentAnim = 9;
			CurrentFrame = 0;
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
			CurrentFrame = AnimFrame % 16;
		}
		else if (State & STATE_GRABBING) {
			CurrentAnim = 12;
			CurrentFrame = 7;
		}
		else {
			CurrentAnim = 0;
			CurrentFrame = AnimFrame % 45;
		}
	}
	DrawSelf(hdc, Scale, CurrentFrame, CurrentAnim);
}

int CoolGiraffe::Size()
{
	return sizeof(*this);
}

void CoolGiraffe::DrawSelf(HDC hdc, Vector2 Scale, int CurrentFrame, int CurrentAnim)
{
	if (State & STATE_INTANGIBLE) {
		SelectObject(hdc, IntangiblePen);
	}
	else {
		SelectObject(hdc, GiraffePen);
	}

	std::vector<POINT> points;
	std::vector<Vector2> vPoints = (*Moves->GetSkelPoints(CurrentAnim, CurrentFrame));

	for (int i = 0; i < vPoints.size(); ++i) {
		points.push_back(Giraffe::VecToPoint(Position + Facing * vPoints[i], Scale));
	}

	//if (points.size() > 40) {
	Polyline(hdc, &points[0], 30);
	PolyBezier(hdc, &points[29], 4);
	Polyline(hdc, &points[32], 5);
	PolyBezier(hdc, &points[36], 4);
	Polyline(hdc, &points[39], 2);
	Polyline(hdc, &points[41], 19);

	Vector2 dir = vPoints[36] - vPoints[35];
	dir.Normalize();
	Vector2 perp = { -dir.y, dir.x };

	for (int i = 0; i < 20; ++i) {
		points[i] = VecToPoint(Position + Facing * (vPoints[36] + i / 20.0f * dir + 0.1f * sinf((AnimFrame + i) / (0.5f * 3.14159f)) * perp), Scale);
	}
	Polyline(hdc, &points[0], 20);
	//}
	//else {
	//	Polyline(hdc, &points[0], 27);
	//	PolyBezier(hdc, &points[26], 4);
	//	Polyline(hdc, &points[29], 5);
	//	PolyBezier(hdc, &points[33], 4);
	//	Polyline(hdc, &points[36], 2);
	//}
}

void CoolGiraffe::DrawHitbox(HDC hdc, Vector2 Scale, Vector2 Pos, float Rad)
{
	SelectObject(hdc, GiraffePen);
	Ellipse(hdc, Scale.x * (Position.x + Facing.x * Pos.x - Rad), Scale.y * (Position.y + Facing.y * Pos.y - Rad), Scale.x * (Position.x + Facing.x * Pos.x + Rad), Scale.y * (Position.y + Facing.y * Pos.y + Rad));
}

void CoolGiraffe::Landing(Stage& stage, const int frameNumber, std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes)
{
	SoundMoveState |= SOUND_JUMPLAND;
	SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_JUMPLAND] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_JUMPLAND);
	if ((State & STATE_HEAVY) && !(State & (STATE_FORWARD | STATE_UP | STATE_BACK))) {
		//State &= ~STATE_JUMPING;
		HasAirDash = true;
		HasDoubleJump = false;
	}
	else {
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

void CoolGiraffe::RecieveHits(Stage& stage, const int frameNumber)
{
	if (incomingGrab) {
		State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_SHIELDING | STATE_DROPSHIELD | STATE_SHIELDSTUN | STATE_JUMPSQUAT | STATE_JUMPLAND | STATE_HITSTUN | STATE_FASTFALL | STATE_DOUBLEJUMPWAIT | STATE_WAVEDASH | STATE_RUNNING | STATE_SHORTHOP | STATE_CROUCH | STATE_TECHATTEMPT | STATE_TECHLAG | STATE_TECHING | STATE_KNOCKDOWN | STATE_KNOCKDOWNLAG | STATE_ROLLING | STATE_GETUPATTACK | STATE_GRABBING | STATE_THROW);
		State |= STATE_GRABBED;
		SoundMoveState &= ~(SOUND_SHIELD | SOUND_RUN);
		incomingGrab = false;
	}

	if (numIncoming > 0) {
		if (State & STATE_SHIELDING) {
			for (int j = 0; j < numIncoming; ++j) {
				State |= STATE_SHIELDSTUN;
				AttackDelay = (int)max(AttackDelay, frameNumber + IncomingHits[j].Damage * 50);
			}
		}
		else if (State & STATE_JUMPING && State & STATE_HEAVY && State & STATE_DOWN && AnimFrame >= 5 && AnimFrame <= 29) {
			AnimFrame = 28;
			AttackDelay = frameNumber + 30;
		}
		else {
			for (int j = 0; j < numIncoming; ++j) {
				Knockback += IncomingHits[j].Damage;
				float KnockbackApplied;
				if (IncomingHits[j].Fixed) {
					KnockbackApplied = IncomingHits[j].Knockback;
				}
				else {
					KnockbackApplied = ((((Knockback / 10 + (Knockback * IncomingHits[j].Damage / 20)) * (200 / (Mass + 100)) * 1.4f + 0.18f) * IncomingHits[j].Scale) + IncomingHits[j].Knockback);
				}
				Velocity += KnockbackApplied * IncomingHits[j].Force;
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
}

void CoolGiraffe::DrawSmallFlame(HDC hdc, Vector2 Scale, Vector2 Elbow, Vector2 Hand)
{
	SelectObject(hdc, FirePen);

	Vector2 dir = Hand - Elbow;
	dir.Normalize();
	Vector2 perp = { -dir.y, dir.x };

	Vector2 controlPoints[11];
	controlPoints[0] = Position + Facing * (Elbow + 0.2f * dir + 0.25f * perp);
	controlPoints[1] = Position + Facing * (Elbow + 0.5f * dir + 0.4f * perp);
	controlPoints[2] = Position + Facing * (Elbow + 0.45f * dir + 0.3f * perp);
	controlPoints[3] = Position + Facing * (Hand + 0.2f * dir + 0.35f * perp);
	controlPoints[4] = Position + Facing * (Hand + 0.15f * dir + 0.2f * perp);
	controlPoints[5] = Position + Facing * (Hand + 0.35f * dir);
	controlPoints[6] = Position + Facing * (Hand + 0.15f * dir - 0.2f * perp);
	controlPoints[7] = Position + Facing * (Hand + 0.2f * dir - 0.35f * perp);
	controlPoints[8] = Position + Facing * (Elbow + 0.45f * dir - 0.3f * perp);
	controlPoints[9] = Position + Facing * (Elbow + 0.5f * dir - 0.4f * perp);
	controlPoints[10] = Position + Facing * (Elbow + 0.2f * dir - 0.25f * perp);

	POINT points[31];
	CoolProjFuncs::Crackle(points, controlPoints, 10, 3, 0.3f, Scale);
	Polyline(hdc, points, 31);
}

void CoolGiraffe::DrawLargeFlame(HDC hdc, Vector2 Scale, Vector2 Neck, Vector2 Head)
{
	SelectObject(hdc, FirePen);

	Vector2 dir = Head - Neck;
	dir.Normalize();
	Vector2 perp = { -dir.y, dir.x };

	Vector2 controlPoints[11];
	controlPoints[0] = Position + Facing * (Head - 2.4f * dir + 0.25f * perp);
	controlPoints[1] = Position + Facing * (Head - 1.0f * dir + 1.4f * perp);
	controlPoints[2] = Position + Facing * (Head - 0.7f * dir + 1.0f * perp);
	controlPoints[3] = Position + Facing * (Head - 0.4f * dir + 1.2f * perp);
	controlPoints[4] = Position + Facing * (Head + 0.3f * dir + 0.8f * perp);
	controlPoints[5] = Position + Facing * (Head + 0.7f * dir);
	controlPoints[6] = Position + Facing * (Head + 0.3f * dir - 0.8f * perp);
	controlPoints[7] = Position + Facing * (Head - 0.4f * dir - 1.2f * perp);
	controlPoints[8] = Position + Facing * (Head - 0.7f * dir - 1.0f * perp);
	controlPoints[9] = Position + Facing * (Head - 1.0f * dir - 1.4f * perp);
	controlPoints[10] = Position + Facing * (Head - 2.4f * dir - 0.25f * perp);

	POINT points[51];
	CoolProjFuncs::Crackle(points, controlPoints, 10, 5, 0.5f, Scale);
	Polyline(hdc, points, 51);
}

