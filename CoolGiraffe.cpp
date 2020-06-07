#include "CoolGiraffe.h"
#include "CoolProjFuncs.h"
#include "NormProjFuncs.h"//TEMP

CoolGiraffe::CoolGiraffe(Vector2 _Position, MoveSet* _Moves, COLORREF _Colour)
{
	//Movement
	Position = _Position;
	Velocity = Vector2(0, 0);
	MaxGroundSpeed = 0.4f;
	MaxAirSpeed = Vector2(0.3f, 0.7f);
	RunAccel = 0.1f;
	AirAccel = 0.03f;
	Gravity = 0.02f;
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

	//Animation
	AnimFrame = 0;
	GiraffePen = CreatePen(PS_SOLID, 1, _Colour);
	IntangiblePen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	ShieldBrush = CreateHatchBrush(HS_BDIAGONAL, RGB(0, 255, 127));
	SpitBrush = CreateSolidBrush(RGB(90, 210, 180));
	ShineBrush = CreateSolidBrush(RGB(0, 255, 255));
}

CoolGiraffe::~CoolGiraffe()
{
}

void CoolGiraffe::Update(std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes, const int num_giraffes, const int i, const int inputs, const int frameNumber, Stage& stage)
{
	++AnimFrame;

	TransitionStates(frameNumber);
	ParseInputs(inputs, frameNumber, stage);

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
		Projectiles.Append(Projectile(Position + Vector2(Facing.x, 0), { 0.1f * Facing.x, 0 }, 1.0f, {Facing.x, 0}, 0.5f, 0.0f, 0.3f, false, LastAttackID, frameNumber + 1000, CoolProjFuncs::StandardOnHit, CoolProjFuncs::StandardUpdate, CoolProjFuncs::FireballDraw, GiraffePen, nullptr));
	}


	if ((State & STATE_HEAVY) && (State & STATE_UP)) {
		//Fire neck
		if (State & STATE_JUMPING) {
			if (AnimFrame == 10) {
				Projectiles.Append(Projectile(Position + Vector2(0.2f, -1.2f), { Facing.x * 0.65f, -0.65f }, 0.3f, { 0.0f, 0.0f }, 0.1f, 0.1f, 1.0f, true, LastAttackID, AttackDelay - 10, NormProjFuncs::NeckGrabOnHit, NormProjFuncs::NeckGrabUpdate, NormProjFuncs::NeckGrabDraw, GiraffePen, SpitBrush));
			}
		}
		//Flip during dp
		else if (AnimFrame == 24) {
			Facing.x *= -1;
		}
	}
	//Fire spit
	else if ((State & STATE_HEAVY) && !(State & (STATE_WEAK | STATE_UP | STATE_FORWARD | STATE_BACK | STATE_DOWN)) && AnimFrame == 13) {
		Projectiles.Append(Projectile(Position + Vector2(0.2f, -1.2f), { Facing.x * 0.5f, 0.0f }, 0.3f, { Facing.x, 0.0f }, 0.1f, 0.1f, 1.0f, true, LastAttackID, frameNumber + 100, NormProjFuncs::SpitOnHit, NormProjFuncs::SpitUpdate, NormProjFuncs::SpitDraw, GiraffePen, SpitBrush));
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

	ApplyChanges(giraffes, num_giraffes, frameNumber, i);
}

void CoolGiraffe::Draw(HDC hdc, Vector2 Scale, int frameNumber)
{
	for (int i = 0; i < Projectiles.Size(); ++i) {
		Projectiles[i].Draw(Projectiles[i], *this, hdc, Scale, frameNumber);
	}

	int CurrentAnim = 0;
	int CurrentFrame = 0;


	if (State & STATE_INTANGIBLE) {
		SelectObject(hdc, IntangiblePen);
	}
	else {
		SelectObject(hdc, GiraffePen);
	}


	if ((State & STATE_HEAVY) && (State & STATE_JUMPING)) {
		if (State & STATE_UP) {
			if (AnimFrame >= 10 && AnimFrame <= 30) {
				POINT points[NUM_POINTS];
				for (int i = 0; i < NUM_POINTS; ++i) {
					points[i] = Giraffe::VecToPoint(Position + Facing * (*Moves->GetSkelPoints(CurrentAnim, CurrentFrame))[i], Scale);
				}
				Polyline(hdc, points, 27);
				Polyline(hdc, &points[36], 2);
			}
			else {
				DrawSelf(hdc, Scale, AnimFrame, AttackNum);
			}
		}
		else if (State & STATE_DOWN) {
			DrawSelf(hdc, Scale, AnimFrame, AttackNum);
			SelectObject(hdc, ShineBrush);
			POINT points[12];
			float r = 2.5f * cosf(AnimFrame / 15.0f * 3.1415f);
			float r1 = 0.5f * r;
			float r2 = 0.866025f * r;
			points[0] = Giraffe::VecToPoint(Position + Vector2(r, 0), Scale);
			points[1] = Giraffe::VecToPoint(Position + Vector2(r1, r2), Scale);
			points[2] = Giraffe::VecToPoint(Position + Vector2(-r1, r2), Scale);
			points[3] = Giraffe::VecToPoint(Position + Vector2(-r, 0), Scale);
			points[4] = Giraffe::VecToPoint(Position + Vector2(-r1, -r2), Scale);
			points[5] = Giraffe::VecToPoint(Position + Vector2(r1, -r2), Scale);

			r *= 0.6f;
			r1 *= 0.6f;
			r2 *= 0.6f;
			points[6] = Giraffe::VecToPoint(Position + Vector2(r, 0), Scale);
			points[7] = Giraffe::VecToPoint(Position + Vector2(r1, r2), Scale);
			points[8] = Giraffe::VecToPoint(Position + Vector2(-r1, r2), Scale);
			points[9] = Giraffe::VecToPoint(Position + Vector2(-r, 0), Scale);
			points[10] = Giraffe::VecToPoint(Position + Vector2(-r1, -r2), Scale);
			points[11] = Giraffe::VecToPoint(Position + Vector2(r1, -r2), Scale);

			Polygon(hdc, points, 12);
		}
		else {
			DrawSelf(hdc, Scale, AnimFrame, AttackNum);
		}
		return;
	}

	if (State & (STATE_WEAK | STATE_HEAVY | STATE_THROW)) {
		CurrentAnim = AttackNum;
		CurrentFrame = AnimFrame;
		/*for (int i = 0; i < numHitboxes; ++i) {
			DrawHitbox(hdc, Scale, (*Hitboxes)[i].Position, (*Hitboxes)[i].Radius);
		}*/
	}
	else {
		if (State & STATE_HITSTUN) {
			CurrentAnim = 6;
			CurrentFrame = AnimFrame % 16;
		}
		else if (State & STATE_SHIELDSTUN) {
			SelectObject(hdc, ShieldBrush);
			DrawHitbox(hdc, Scale, { 0,0 }, 2.5f);
			//Ellipse(hdc, (Position.x - 2.5f) * Scale.x, (Position.y - 2.5f) * Scale.y, (Position.x + 2.5f) * Scale.x, (Position.y + 2.5f) * Scale.y);
			CurrentAnim = 6;
			CurrentFrame = AnimFrame % 16;
		}
		else if (State & STATE_SHIELDING) {
			SelectObject(hdc, ShieldBrush);
			DrawHitbox(hdc, Scale, { 0,0 }, 2.5f);
			//Ellipse(hdc, (Position.x - 2.5f) * Scale.x, (Position.y - 2.5f) * Scale.y, (Position.x + 2.5f) * Scale.x, (Position.y + 2.5f) * Scale.y);
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

void CoolGiraffe::DrawSelf(HDC hdc, Vector2 Scale, int CurrentFrame, int CurrentAnim)
{
	std::vector<POINT> points;
	std::vector<Vector2> vPoints = (*Moves->GetSkelPoints(CurrentAnim, CurrentFrame));

	for (int i = 0; i < vPoints.size(); ++i) {
		points.push_back(Giraffe::VecToPoint(Position + Facing * vPoints[i], Scale));
	}

	if (points.size() > 40) {
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
	}
	else {
		Polyline(hdc, &points[0], 27);
		PolyBezier(hdc, &points[26], 4);
		Polyline(hdc, &points[29], 5);
		PolyBezier(hdc, &points[33], 4);
		Polyline(hdc, &points[36], 2);
	}
}

void CoolGiraffe::DrawHitbox(HDC hdc, Vector2 Scale, Vector2 Pos, float Rad)
{
	Ellipse(hdc, Scale.x * (Position.x + Facing.x * Pos.x - Rad), Scale.y * (Position.y + Facing.y * Pos.y - Rad), Scale.x * (Position.x + Facing.x * Pos.x + Rad), Scale.y * (Position.y + Facing.y * Pos.y + Rad));
}

