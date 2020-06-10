#include "RobotGiraffe.h"
#include "RobotProjFuncs.h"

RobotGiraffe::RobotGiraffe(Vector2 _Position, COLORREF _Colour)
{
	//Movement
	Position = _Position;
	Velocity = Vector2(0, 0);
	MaxGroundSpeed = 0.25f;
	MaxAirSpeed = Vector2(0.2f, 0.7f);
	RunAccel = 0.03f;
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
	Moves = new RobotMoveSet();
	Moves->InitMoves();
	Moves->InitThrows();
	Moves->InitTilts();
	Moves->InitSmashes();
	Moves->InitAerials();
	Moves->InitSpecials();

	//Misc
	Stocks = MAX_STOCKS;
	Knockback = 0;
	Mass = 150;
	CommandGrabPointer = 0;
	Charge = 0;
	BigLaser = false;
	HasSword = true;
	SwordDelay = 0;

	//Animation
	AnimFrame = 0;
	GiraffePen = CreatePen(PS_SOLID, 1, _Colour);
	IntangiblePen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	ShieldBrush = CreateHatchBrush(HS_BDIAGONAL, RGB(0, 255, 127));
	//FirePen = CreatePen(PS_SOLID, 1, RGB(230, 128, 40));
	LaserPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
	LanceBrush = CreateSolidBrush(RGB(0, 0, 0));
	RedBrush = CreateSolidBrush(RGB(255, 0, 0));
}

RobotGiraffe::~RobotGiraffe()
{
	delete Moves;
}

void RobotGiraffe::Update(std::array<Giraffe*, 4> giraffes, const int num_giraffes, const int i, const int inputs, const int frameNumber, Stage& stage)
{
	++AnimFrame;
	++Charge;

	TransitionStates(frameNumber);
	if (HasSword == false && frameNumber >= SwordDelay) {
		HasSword = true;
	}

	ParseInputs(inputs, frameNumber, stage);
	UniqueChanges(giraffes, num_giraffes, i, inputs, frameNumber, stage);
	ApplyChanges(giraffes, num_giraffes, frameNumber, i);
}

void RobotGiraffe::UniqueChanges(std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes, const int num_giraffes, const int i, const int inputs, const int frameNumber, Stage& stage)
{
	//Grab
	if (State & STATE_WEAK && State & STATE_HEAVY && AnimFrame == 11) {
		Projectiles.Append(Projectile(Position + Vector2(Facing.x * 0.5f, 0), { Facing.x * 0.6f, 0 }, 0.5f, { 0,0 }, 0, 0, 0, true, LastAttackID, frameNumber + 15, RobotProjFuncs::GrabberOnHit, RobotProjFuncs::StandardUpdate, RobotProjFuncs::GrabberDraw, GiraffePen, ShieldBrush));
	}

	//Character Move-Specific Updates
	if ((State & STATE_JUMPING) && (State & STATE_HEAVY) && (State & STATE_FORWARD) && AnimFrame == 7) {
		Collider grabCol = { {Position.x + 1.885000f * Facing.x, Position.y - 0.650000f}, 0.855862f };
		for (int j = 0; j < num_giraffes; ++j) {
			if (j != i && giraffes[j]->Stocks > 0) {
				if (giraffes[j]->GrabHit(grabCol, Facing, frameNumber)) {
					State &= ~(STATE_WEAK | STATE_HEAVY | STATE_RUNNING);
					State |= STATE_GRABBING;
					TechDelay = frameNumber + 30;
					giraffes[j]->Velocity = Velocity;
					giraffes[j]->State |= STATE_JUMPING;
					CommandGrabPointer = j;
					break;
				}
			}
		}
	}


	//Throw lance
	if ((State & STATE_WEAK) && (State & STATE_JUMPING) && (State & STATE_BACK) && AnimFrame == 10) {
		Projectiles.Append(Projectile(Position + Vector2(0, -1.7f), { Facing.x * -0.8f, 0.0f }, 0.3f, { -Facing.x, 0.0f }, 1.6f, 0.3f, 0.3f, false, LastAttackID, frameNumber + 50, RobotProjFuncs::StandardOnHit, RobotProjFuncs::StandardUpdate, RobotProjFuncs::LanceDraw, GiraffePen, LanceBrush));
	}
	//Fire Missile
	else if ((State & STATE_WEAK) && (State & STATE_JUMPING) &&!(State & (STATE_UP | STATE_FORWARD | STATE_BACK | STATE_DOWN)) && AnimFrame >= 7 && AnimFrame <= 15 && AnimFrame % 2 == 0) {
		if (AnimFrame % 4 == 0) {
			Projectiles.Append(Projectile(Position + Vector2(2.0f, -2.2f), { Facing.x * 0.5f, 0.0f }, 0.3f, { Facing.x, 0.0f }, 0.3f, 0.2f, 0.3f, false, LastAttackID++, frameNumber + 50, RobotProjFuncs::MissileOnHit, RobotProjFuncs::MissileUpdate, RobotProjFuncs::MissileDraw, GiraffePen, nullptr));
		}
		else {
			Projectiles.Append(Projectile(Position + Vector2(-2.0f, -2.2f), { Facing.x * 0.5f, 0.0f }, 0.3f, { Facing.x, 0.0f }, 0.3f, 0.2f, 0.3f, false, LastAttackID++, frameNumber + 50, RobotProjFuncs::MissileOnHit, RobotProjFuncs::MissileUpdate, RobotProjFuncs::MissileDraw, GiraffePen, nullptr));
		}
	}

	if (State & STATE_HEAVY) {
		//UpB
		if ((State & STATE_JUMPING) && (State & STATE_UP) && AnimFrame >= 13 && AnimFrame <= 23) {
			if (AnimFrame == 14) {
				Velocity.y = 0;
			}
			Velocity += Vector2(0.02f * Facing.x, -0.2f);
		}
		//Drop Bomb
		else if ((State & STATE_JUMPING) && (State & STATE_DOWN) && AnimFrame == 20) {
			Projectiles.Append(Projectile(Position, { 0, 0 }, 0.5f, { Facing.x, -1.0f }, 1.5f, 0.7f, 0.3f, false, LastAttackID++, frameNumber + 50, RobotProjFuncs::BombOnHit, RobotProjFuncs::BombUpdate, RobotProjFuncs::BombDraw, GiraffePen, nullptr));
		}
		//Throw sword
		else if ((State & STATE_JUMPING) && (State & STATE_FORWARD) && AnimFrame == 10 && HasSword) {
			Projectiles.Append(Projectile(Position, { 0.5f * Facing.x, 0 }, 1.5f, {Facing.x, 0.5f}, 0.5f, 0.5f, 0.5f, false, LastAttackID, frameNumber + 50, RobotProjFuncs::SwordOnHit, RobotProjFuncs::SwordUpdate, RobotProjFuncs::SwordDraw, LaserPen, nullptr));
			HasSword = false;
			SwordDelay = frameNumber + 100;
		}
		//Laser
		else if (!(State & (STATE_WEAK | STATE_UP | STATE_FORWARD | STATE_BACK | STATE_DOWN))) {
			if (AnimFrame == 0) {
				BigLaser = Charge > CHARGELIMIT;
				if (BigLaser) {
					Charge = 0;
				}
			}
			if (!BigLaser && AnimFrame == 7) {
				Projectiles.Append(Projectile(Position + Vector2(0.5, -0.5f), { Facing.x, 0.0f }, 0.3f, { Facing.x, 0.0f }, 0.1f, 0.1f, 0.0f, true, LastAttackID, frameNumber + 50, RobotProjFuncs::StandardOnHit, RobotProjFuncs::StandardUpdate, RobotProjFuncs::SmallLaserDraw, LaserPen, nullptr));
			}
			else if (BigLaser && AnimFrame >= 7) {
				Projectiles.Append(Projectile(Position + Vector2(0.5f, -1.0f), { 5 * Facing.x, -3.0f }, 1.0f, { Facing.x, -1.0f }, 2.0f, 1.0f, 1.0f, false, LastAttackID++, frameNumber + 50, RobotProjFuncs::StandardOnHit, RobotProjFuncs::StandardUpdate, RobotProjFuncs::BigLaserDraw, LaserPen, nullptr));
				if (State & STATE_JUMPING) {
					Velocity = { 0, -Gravity - 0.00001f };
				}
			}
		}
	}
}

void RobotGiraffe::Draw(HDC hdc, Vector2 Scale, int frameNumber)
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

	if (State & STATE_HEAVY) {
		DrawSelf(hdc, Scale, AnimFrame, AttackNum);
		if ((State & STATE_JUMPING)) {
			if ((State & STATE_UP) && AnimFrame >= 13 && AnimFrame <= 23) {
				DrawBlast(hdc, Scale, Vector2(0, 1), Vector2(0, 1.5f));
				return;
			}
			else if ((State & STATE_FORWARD) && AnimFrame <= 9 && HasSword) {
				DrawBeamSword(hdc, Scale, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[25] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[30]) * 0.5f, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[27] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[28]) * 0.5f);
				return;
			}
		}
		else {
			if ((State & STATE_FORWARD) && (AnimFrame >= 10 && AnimFrame <= 13)) {
				DrawBlast(hdc, Scale, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[25] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[30]) * 0.5f, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[27] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[28]) * 0.5f);
				return;
			}
			else if ((State & STATE_UP) && ((AnimFrame >= 12 && AnimFrame <= 14) || (AnimFrame >= 18 && AnimFrame <= 20) || (AnimFrame >= 24 && AnimFrame <= 26))) {
				DrawBlast(hdc, Scale, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[25] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[30]) * 0.5f, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[27] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[28]) * 0.5f);
				return;
			}
			else if (State & STATE_DOWN && (AnimFrame >= 19 && AnimFrame <= 22)) {
				DrawBlast(hdc, Scale, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[25] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[30]) * 0.5f, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[27] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[28]) * 0.5f);
				return;
			}
		}
		return;
	}

	if (State & STATE_WEAK) {
		DrawSelf(hdc, Scale, AnimFrame, AttackNum);
		if (State & STATE_JUMPING) {
			if (State & (STATE_DOWN | STATE_BACK)) {
				return;
			}
			if (State & STATE_FORWARD) {
				if (AnimFrame <= 4 || AnimFrame >= 24 || !HasSword) {
					DrawSword(hdc, Scale, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[25] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[30]) * 0.5f, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[27] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[28]) * 0.5f);
					return;
				}
				else {
					DrawBeamSword(hdc, Scale, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[25] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[30]) * 0.5f, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[27] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[28]) * 0.5f);
					return;
				}
			}
			else if (State & STATE_UP) {
				DrawDrill(hdc, Scale, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[25] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[30]) * 0.5f, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[27] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[28]) * 0.5f);
			}
			else {
				if (AnimFrame >= 7 && AnimFrame <= 25) {
					DrawML(hdc, Scale, (((*Moves->GetSkelPoints(AttackNum, AnimFrame))[25] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[30]) * 0.5f - (*Moves->GetSkelPoints(AttackNum, AnimFrame))[33]), (*Moves->GetSkelPoints(AttackNum, AnimFrame))[33]);
				}
				return;
			}
		}
		else {
			if (State & STATE_FORWARD && (AnimFrame >= 5 && AnimFrame <= 20)) {
				DrawAxe(hdc, Scale, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[25] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[30]) * 0.5f, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[27] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[28]) * 0.5f);
				return;
			}
			else if (State & STATE_UP && (AnimFrame >= 14)) {
				DrawMace(hdc, Scale, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[25] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[30]) * 0.5f, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[27] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[28]) * 0.5f);
				return;
			}
			else if (State & STATE_DOWN && (AnimFrame >= 5 && AnimFrame <= 20)) {
				DrawSword(hdc, Scale, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[25] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[30]) * 0.5f, ((*Moves->GetSkelPoints(AttackNum, AnimFrame))[27] + (*Moves->GetSkelPoints(AttackNum, AnimFrame))[28]) * 0.5f);
				return;
			}
			else {
				return;
			}
		}
	}

	if (State & (STATE_WEAK | STATE_HEAVY | STATE_THROW)) {
		CurrentAnim = AttackNum;
		CurrentFrame = AnimFrame;
	}
	else {
		if (State & STATE_HITSTUN) {
			CurrentAnim = 6;
			CurrentFrame = AnimFrame % 8;
		}
		else if (State & STATE_SHIELDSTUN) {
			SelectObject(hdc, ShieldBrush);
			DrawHitbox(hdc, Scale, { 0,0 }, 2.5f);
			CurrentAnim = 6;
			CurrentFrame = AnimFrame % 8;
		}
		else if (State & STATE_SHIELDING) {
			SelectObject(hdc, ShieldBrush);
			DrawHitbox(hdc, Scale, { 0,0 }, 2.5f);
			CurrentAnim = 0;
			CurrentFrame = 0;
		}
		else if (State & STATE_RUNNING) {
			CurrentAnim = 1;
			CurrentFrame = AnimFrame % 2;
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
			CurrentFrame = AnimFrame % 8;
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
			CurrentFrame = AnimFrame % 8;
		}
		else if (State & STATE_GRABBING) {
			CurrentAnim = 12;
			CurrentFrame = 7;
		}
	}
	DrawSelf(hdc, Scale, CurrentFrame, CurrentAnim);
}

int RobotGiraffe::Size()
{
	return sizeof(*this);
}

void RobotGiraffe::DrawSelf(HDC hdc, Vector2 Scale, int CurrentFrame, int CurrentAnim)
{
	SelectObject(hdc, GiraffePen);
	std::vector<POINT> points;
	std::vector<Vector2> vPoints = (*Moves->GetSkelPoints(CurrentAnim, CurrentFrame));

	for (int i = 0; i < vPoints.size(); ++i) {
		points.push_back(Giraffe::VecToPoint(Position + Facing * vPoints[i], Scale));
	}
	Polyline(hdc, &points[0], points.size());

	if (Charge > CHARGELIMIT) {
		SelectObject(hdc, RedBrush);
		Vector2 EyePos = { vPoints[26].x + vPoints[27].x, vPoints[27].y + vPoints[28].y };
		EyePos *= 0.5f;
		float Radius = 0.2f;
		DrawHitbox(hdc, Scale, EyePos, Radius);
	}
}

void RobotGiraffe::DrawHitbox(HDC hdc, Vector2 Scale, Vector2 Pos, float Rad)
{
	SelectObject(hdc, GiraffePen);
	Ellipse(hdc, (int)(Scale.x * (Position.x + Facing.x * Pos.x - Rad)), (int)(Scale.y * (Position.y + Facing.y * Pos.y - Rad)), (int)(Scale.x * (Position.x + Facing.x * Pos.x + Rad)), (int)(Scale.y * (Position.y + Facing.y * Pos.y + Rad)));
}

void RobotGiraffe::DrawAxe(HDC hdc, Vector2 Scale, Vector2 Neck, Vector2 Head)
{
	Vector2 dir = Head - Neck;
	dir.Normalize();
	Vector2 perp = { -dir.y, dir.x };
	Vector2 Pos = Head + dir;

	POINT points[8];
	points[0] = Giraffe::VecToPoint(Position + Facing * Head, Scale);
	points[1] = Giraffe::VecToPoint(Position + Facing * Pos, Scale);
	points[2] = Giraffe::VecToPoint(Position + Facing * (Pos + 0.3f * perp + -0.3f * dir), Scale);
	points[3] = Giraffe::VecToPoint(Position + Facing * (Pos + 0.3f * perp + 0.4f * dir), Scale);
	points[4] = Giraffe::VecToPoint(Position + Facing * (Pos + 0.1f * dir), Scale);
	points[5] = Giraffe::VecToPoint(Position + Facing * (Pos + -0.3f * perp + 0.4f * dir), Scale);
	points[6] = Giraffe::VecToPoint(Position + Facing * (Pos + -0.3f * perp + -0.3f * dir), Scale);
	points[7] = Giraffe::VecToPoint(Position + Facing * Pos, Scale);

	Polyline(hdc, points, 8);
}

void RobotGiraffe::DrawSword(HDC hdc, Vector2 Scale, Vector2 Neck, Vector2 Head)
{
	Vector2 dir = Head - Neck;
	dir.Normalize();
	Vector2 perp = { -dir.y, dir.x };
	Vector2 Pos = Head + dir * 0.3f;

	POINT points[6];
	points[0] = Giraffe::VecToPoint(Position + Facing * Head, Scale);
	points[1] = Giraffe::VecToPoint(Position + Facing * Pos, Scale);
	points[2] = Giraffe::VecToPoint(Position + Facing * (Pos + perp * 0.2f), Scale);
	points[3] = Giraffe::VecToPoint(Position + Facing * (Pos - perp * 0.2f), Scale);
	points[4] = Giraffe::VecToPoint(Position + Facing * Pos, Scale);
	points[5] = Giraffe::VecToPoint(Position + Facing * (Head + dir * 1.5f), Scale);

	Polyline(hdc, points, 6);
}

void RobotGiraffe::DrawMace(HDC hdc, Vector2 Scale, Vector2 Neck, Vector2 Head)
{
	Vector2 dir = Head - Neck;
	dir.Normalize();
	Vector2 perp = { -dir.y, dir.x };
	Vector2 Pos = Head + dir;

	POINT points[11];

	points[0] = Giraffe::VecToPoint(Position + Facing * Head, Scale);
	points[1] = Giraffe::VecToPoint(Position + Facing * Pos, Scale);
	points[2] = Giraffe::VecToPoint(Position + Facing * (Pos + perp * 0.3f), Scale);
	points[3] = Giraffe::VecToPoint(Position + Facing * (Pos + perp * 0.3f + dir * 0.6f), Scale);
	points[4] = Giraffe::VecToPoint(Position + Facing * (Pos + perp * -0.3f + dir * 0.6f), Scale);
	points[5] = Giraffe::VecToPoint(Position + Facing * (Pos + perp * -0.3f), Scale);
	points[6] = Giraffe::VecToPoint(Position + Facing * Pos, Scale);
	points[7] = Giraffe::VecToPoint(Position + Facing * (Pos + perp * 0.3f + dir * 0.3f), Scale);
	points[8] = Giraffe::VecToPoint(Position + Facing * (Pos + dir * 0.6f), Scale);
	points[9] = Giraffe::VecToPoint(Position + Facing * (Pos + perp * -0.3f + dir * 0.3f), Scale);
	points[10] = Giraffe::VecToPoint(Position + Facing * Pos, Scale);

	Polyline(hdc, points, 11);
}

void RobotGiraffe::DrawBlast(HDC hdc, Vector2 Scale, Vector2 Neck, Vector2 Head)
{
	SelectObject(hdc, LaserPen);
	Vector2 dir = Head - Neck;
	dir.Normalize();
	Vector2 perp = { -dir.y, dir.x };
	Vector2 Pos = Head + dir;

	Vector2 controlPoints[11];

	controlPoints[0] = Position + Facing * (Head + perp * 0.2f);
	controlPoints[1] = Position + Facing * (Head + dir * 1.0f + perp * 1.5f);
	controlPoints[2] = Position + Facing * (Head + dir * 0.5f + perp * 0.35f);
	controlPoints[3] = Position + Facing * (Head + dir * 1.5f + perp * 0.75f);
	controlPoints[4] = Position + Facing * (Head + dir * 0.65f + perp * 0.25f);
	controlPoints[5] = Position + Facing * (Head + dir * 2.0f);
	controlPoints[6] = Position + Facing * (Head + dir * 0.65f + perp * -0.25f);
	controlPoints[7] = Position + Facing * (Head + dir * 1.5f + perp * -0.75f);
	controlPoints[8] = Position + Facing * (Head + dir * 0.5f + perp * -0.35f);
	controlPoints[9] = Position + Facing * (Head + dir * 1.0f + perp * -1.5f);
	controlPoints[10] = Position + Facing * (Head + perp * -0.2f);

	POINT points[101];
	RobotProjFuncs::Crackle(points,controlPoints, 10, 10, 0.1f, Scale);
	Polyline(hdc, points, 101);
}

void RobotGiraffe::DrawBeamSword(HDC hdc, Vector2 Scale, Vector2 Neck, Vector2 Head)
{
	DrawSword(hdc, Scale, Neck, Head);
	
	SelectObject(hdc, LaserPen);
	Vector2 dir = Head - Neck;
	dir.Normalize();
	Vector2 perp = { -dir.y, dir.x };
	Vector2 Pos = Head + dir * 0.3f;

	Vector2 controlPoints[7];
	controlPoints[0] = Position + Facing * (Pos + perp * 0.2f);
	controlPoints[1] = Position + Facing * (Pos + dir * 0.5f + perp * 0.3f);
	controlPoints[2] = Position + Facing * (Pos + dir * 2.0f + perp * 0.2f);
	controlPoints[3] = Position + Facing * (Pos + dir * 2.5f);
	controlPoints[4] = Position + Facing * (Pos + dir * 2.0f + perp * -0.2f);
	controlPoints[5] = Position + Facing * (Pos + dir * 0.5f + perp * -0.3f);
	controlPoints[6] = Position + Facing * (Pos + perp * -0.2f);

	POINT points[61];
	RobotProjFuncs::Crackle(points, controlPoints, 6, 10, 0.2f, Scale);
	Polyline(hdc, points, 61);
}

void RobotGiraffe::DrawML(HDC hdc, Vector2 Scale, Vector2 dir, Vector2 Body)
{
	SelectObject(hdc, CreateSolidBrush(0));
	dir.Normalize();
	Vector2 perp = { -dir.y, dir.x };
	Vector2 up = { 0, -1 };
	Vector2 right = { 1, 0 };

	POINT points[6];

	points[0] = Giraffe::VecToPoint(Position + Facing * (Body + 0.5f * dir + 0.2f * perp), Scale);
	points[1] = Giraffe::VecToPoint(Position + Facing * (Body + dir + perp), Scale);
	points[2] = Giraffe::VecToPoint(Position + Facing * (Body + dir + perp + 1.3f * up), Scale);
	points[3] = Giraffe::VecToPoint(Position + Facing * (Body + dir + perp + 1.3f * (up + right)), Scale);
	points[4] = Giraffe::VecToPoint(Position + Facing * (Body + dir + perp + 1.3f * right), Scale);
	points[5] = Giraffe::VecToPoint(Position + Facing * (Body + dir + perp), Scale);
	Polyline(hdc, points, 6);

	points[0] = Giraffe::VecToPoint(Position + Facing * (Body + 0.5f * dir + -0.2f * perp), Scale);
	points[1] = Giraffe::VecToPoint(Position + Facing * (Body + dir - perp), Scale);
	points[2] = Giraffe::VecToPoint(Position + Facing * (Body + dir - perp + 1.3f * up), Scale);
	points[3] = Giraffe::VecToPoint(Position + Facing * (Body + dir - perp + 1.3f * (up - right)), Scale);
	points[4] = Giraffe::VecToPoint(Position + Facing * (Body + dir - perp - 1.3f * right), Scale);
	points[5] = Giraffe::VecToPoint(Position + Facing * (Body + dir - perp), Scale);
	Polyline(hdc, points, 6);

	for (int i = 0; i < 2; ++i) {
		for(int j = 0; j < 2; ++j) {
			DrawHitbox(hdc, Scale, Body + dir + perp + (0.17f + 0.5f * i) * 1.3f * up + (0.26f + 0.5f * j) * 1.3f * right, 0.2f);
			DrawHitbox(hdc, Scale, Body + dir - perp + (0.25f + 0.5f * i) * 1.3f * up - (0.24f + 0.5f * j) * 1.3f * right, 0.2f);
		}
	}
}

void RobotGiraffe::DrawDrill(HDC hdc, Vector2 Scale, Vector2 Neck, Vector2 Head)
{
	Vector2 dir = Head - Neck;
	dir.Normalize();
	Vector2 perp = { -dir.y, dir.x };

	POINT points[16];
	points[0] = VecToPoint(Position + Facing * (Head + 0.7f * dir + 0.8f * perp), Scale);
	points[1] = VecToPoint(Position + Facing * (Head + 0.5f * dir + 0.7f * perp), Scale);
	points[2] = VecToPoint(Position + Facing * (Head + 0.3f * dir + 0.6f * perp), Scale);
	points[3] = VecToPoint(Position + Facing * (Head + 0.1f * dir), Scale);
	points[4] = VecToPoint(Position + Facing * (Head + 0.3f * dir + -0.6f * perp), Scale);
	points[5] = VecToPoint(Position + Facing * (Head + 0.5f * dir + -0.7f * perp), Scale);
	points[6] = VecToPoint(Position + Facing * (Head + 0.9f * dir + -0.8f * perp), Scale);
	PolyBezier(hdc, points, 7);

	points[0] = VecToPoint(Position + Facing * (Head + 0.7f * dir + 0.6f * perp), Scale);
	points[1] = VecToPoint(Position + Facing * (Head + 0.6f * dir + 0.5f * perp), Scale);
	points[2] = VecToPoint(Position + Facing * (Head + 0.5f * dir + 0.4f * perp), Scale);
	points[3] = VecToPoint(Position + Facing * (Head + 0.3f * dir), Scale);
	points[4] = VecToPoint(Position + Facing * (Head + 0.5f * dir + -0.4f * perp), Scale);
	points[5] = VecToPoint(Position + Facing * (Head + 0.7f * dir + -0.5f * perp), Scale);
	points[6] = VecToPoint(Position + Facing * (Head + 0.9f * dir + -0.6f * perp), Scale);
	PolyBezier(hdc, points, 7);
	
	points[1] = VecToPoint(Position + Facing * (Head + 0.7f * dir + 0.8f * perp), Scale);
	Polyline(hdc, points, 2);

	//Handle
	points[0] = VecToPoint(Position + Facing * (Head + 0.7f * dir - 0.8f * perp), Scale);
	//Up to base
	points[1] = VecToPoint(Position + Facing * (Head + 1.1f * dir - 0.8f * perp), Scale);
	//Across
	points[2] = VecToPoint(Position + Facing * (Head + 1.1f * dir + 0.8f * perp), Scale);
	//To top
	points[3] = VecToPoint(Position + Facing * (Head + 2.6f * dir), Scale);
	//To base
	points[4] = VecToPoint(Position + Facing * (Head + 1.1f * dir - 0.8f * perp), Scale);

	//Up1
	points[5] = VecToPoint(Position + Facing * (Head + 1.1f * dir + (0.8f - 0.04f * (AnimFrame % 5)) * perp), Scale);
	//Across1
	points[6] = VecToPoint(Position + Facing * (Head + (1.1f + 0.075f * (AnimFrame % 5)) * dir - (0.8f - 0.04f * (AnimFrame % 5)) * perp), Scale);

	//Up2
	points[7] = VecToPoint(Position + Facing * (Head + (1.475f + 0.075f * (AnimFrame % 5)) * dir - (0.6f - 0.04f * (AnimFrame % 5)) * perp), Scale);
	//Across2
	points[8] = VecToPoint(Position + Facing * (Head + (1.1f + 0.075f * (AnimFrame % 5)) * dir + (0.8f - 0.04f * (AnimFrame % 5)) * perp), Scale);

	//Up3
	points[9] = VecToPoint(Position + Facing * (Head + (1.475f + 0.075f * (AnimFrame % 5)) * dir + (0.6f - 0.04f * (AnimFrame % 5)) * perp), Scale);
	//Across3
	points[10] = VecToPoint(Position + Facing * (Head + (1.85f + 0.075f * (AnimFrame % 5)) * dir - (0.4f - 0.04f * (AnimFrame % 5)) * perp), Scale);

	//Up4
	points[11] = VecToPoint(Position + Facing * (Head + (2.225f + 0.075f * (AnimFrame % 5)) * dir - (0.2f - 0.04f * (AnimFrame % 5)) * perp), Scale);
	//Across4
	points[12] = VecToPoint(Position + Facing * (Head + (1.85f + 0.075f * (AnimFrame % 5)) * dir + (0.4f - 0.04f * (AnimFrame % 5)) * perp), Scale);


	//Down to base
	points[13] = VecToPoint(Position + Facing * (Head + 1.1f * dir + 0.8f * perp), Scale);
	//Down to handle
	points[14] = VecToPoint(Position + Facing * (Head + 0.8f * dir + 0.8f * perp), Scale);
	//Close handle
	points[15] = VecToPoint(Position + Facing * (Head + 0.8f * dir - 0.6f * perp), Scale);

	Polyline(hdc, points, 16);
}
