#include "NormGiraffe.h"

NormGiraffe::NormGiraffe(Vec2 _Position, MoveSet* _Moves)
{
	//Movement
	Position = _Position;
	Velocity = Vec2(0, 0);
	MaxGroundSpeed = 0.4f;
	MaxAirSpeed = Vec2(0.3f, 0.7f);
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
	Fullbody = { Vec2(0.0f,0.0f), 2.5f };
	StageCollider = { Vec2(0.0f,0.7f), 1.0f };
	Hitboxes = nullptr;
	Hurtboxes = nullptr;
	numHitboxes = 0;
	numIncoming = 0;
	PrevHitQueue = ArrayQueue();
	LastAttackID = 0;

	//State
	State = 0;
	JumpDelay = 0;
	MaxJumpDelay = 4;
	AttackDelay = 0;
	AttackNum = 0;
	MaxShieldDelay = 5;
	TechDelay = 0;
	Moves = _Moves;

	/*std::ofstream file("NormalizedHitboxes.txt");
	Vec2 normForce;
	for (int i = 8; i < 20; ++i) {
		file << "{";
		for (int f = 0; f < Moves->Hitboxes[i].size(); ++f) {
			file << "{";
			for (int v = 0; v < Moves->Hitboxes[i][f].size(); ++v) {
				normForce = Moves->Hitboxes[i][f][v].Force;
				normForce = normForce.Normalise();
				file << "HitCollider(" << "{" << Moves->Hitboxes[i][f][v].Position.x << "f," << Moves->Hitboxes[i][f][v].Position.y << "f}," << Moves->Hitboxes[i][f][v].Radius << "f,{" << normForce.x << "f," << normForce.y << "f}," << Moves->Hitboxes[i][f][v].Damage << "f),";
			}
			file << "},";
		}
		file << "};\n";
	}
	file.close();*/

	//Misc
	Stocks = 3;
	Knockback = 0;
	Mass = 100;

	//Animation
	AnimFrame = 0;
}

NormGiraffe::~NormGiraffe()
{
}

void NormGiraffe::Update(std::array<Giraffe*, 4> giraffes, const int num_giraffes, const int i, const int inputs, const int frameNumber, Stage& stage)
{
	++AnimFrame;

	//Transition States Based On Frame Number
	if (State & STATE_JUMPSQUAT && frameNumber >= JumpDelay) {
		if (State & STATE_SHORTHOP) {
			Velocity.y = -0.5f * JumpSpeed;
		}
		else {
			Velocity.y = -JumpSpeed;
		}
		State &= ~(STATE_JUMPSQUAT | STATE_SHORTHOP);
		State |= STATE_JUMPING | STATE_DOUBLEJUMPWAIT;
		JumpDelay = frameNumber + MaxJumpDelay * 2;
	}
	else if (State & STATE_JUMPLAND && frameNumber >= JumpDelay) {
		State &= ~STATE_JUMPLAND;
	}
	else if (State & STATE_DOUBLEJUMPWAIT && frameNumber >= JumpDelay) {
		HasDoubleJump = true;
		State &= ~STATE_DOUBLEJUMPWAIT;
	}
	else if (State & STATE_WAVEDASH && frameNumber >= JumpDelay) {
		State &= ~STATE_WAVEDASH;
	}
	if (State & (STATE_WEAK | STATE_HEAVY) && frameNumber >= AttackDelay) {
		State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY);
		numHitboxes = 0;
	}
	else if (State & STATE_HITSTUN && frameNumber >= AttackDelay) {
		State &= ~STATE_HITSTUN;
	}
	else if (State & STATE_ATTACKSTUN && frameNumber >= AttackDelay) {
		State &= ~STATE_ATTACKSTUN;
	}
	else if (State & STATE_SHIELDSTUN && frameNumber >= AttackDelay) {
		State &= ~STATE_SHIELDSTUN;
	}
	else if (State & STATE_DROPSHIELD && frameNumber >= AttackDelay) {
		State &= ~STATE_DROPSHIELD;
	}
	if (State & STATE_TECHATTEMPT && frameNumber >= TechDelay) {
		State &= ~STATE_TECHATTEMPT;
		State |= STATE_TECHLAG;
		TechDelay = frameNumber + 40;
	}
	else if (State & STATE_TECHLAG && frameNumber >= TechDelay) {
		State &= ~STATE_TECHLAG;
	}
	else if (State & STATE_KNOCKDOWNLAG && frameNumber >= TechDelay) {
		State &= ~STATE_KNOCKDOWNLAG;
		State |= STATE_KNOCKDOWN;
	}
	
	//Read Inputs
	if (!(State & (STATE_WEAK | STATE_HEAVY | STATE_SHIELDING | STATE_JUMPSQUAT | STATE_JUMPLAND | STATE_DROPSHIELD | STATE_HITSTUN | STATE_ATTACKSTUN | STATE_KNOCKDOWN | STATE_KNOCKDOWNLAG))) {
		if (!(State & STATE_LEDGEHOG)) {
			if (inputs & INPUT_LEFT) {
				if (State & STATE_JUMPING) {
					if (Velocity.x > -MaxAirSpeed.x) {
						Velocity.x -= AirAccel;
					}
				}
				else if (Velocity.x > -MaxGroundSpeed) {
					Velocity.x -= RunAccel;
					State &= ~STATE_CROUCH;
					State |= STATE_RUNNING;
					Facing = { -1, 1 };
				}
			}
			else if (inputs & INPUT_RIGHT) {
				if (State & STATE_JUMPING) {
					if (Velocity.x < MaxAirSpeed.x) {
						Velocity.x += AirAccel;
					}
				}
				else if (Velocity.x < MaxGroundSpeed) {
					Velocity.x += RunAccel;
					State &= ~STATE_CROUCH;
					State |= STATE_RUNNING;
					Facing = { 1, 1 };
				}
			}
			else {
				State &= ~STATE_RUNNING;
			}
		}
		if (inputs & INPUT_DOWN && !(inputs & (INPUT_WEAK | INPUT_HEAVY))) {
			if (State & STATE_JUMPING) {
				State |= STATE_FASTFALL;
			}
			else if (State & STATE_LEDGEHOG) {
				State &= ~STATE_LEDGEHOG;
				stage.Ledges[LedgeID].Hogged = false;
				State |= STATE_JUMPING | STATE_DOUBLEJUMPWAIT;
				JumpDelay = frameNumber + MaxJumpDelay * 2;
			}
			else {
				State |= STATE_CROUCH;
			}
		}
		else if (!(inputs & INPUT_DOWN) && State & STATE_CROUCH) {
			State &= ~STATE_CROUCH;
		}
	}
	if (inputs & INPUT_WEAK && !(State & (STATE_WEAK | STATE_HEAVY | STATE_SHIELDING | STATE_DROPSHIELD | STATE_JUMPSQUAT | STATE_JUMPLAND | STATE_HITSTUN | STATE_WAVEDASH | STATE_ATTACKSTUN | STATE_LEDGEHOG | STATE_KNOCKDOWNLAG))) {
		State |= STATE_WEAK;
		State &= ~STATE_CROUCH;
		if ((inputs & INPUT_RIGHT && Facing.x == 1) || (inputs & INPUT_LEFT && Facing.x == -1)) {
			State |= STATE_FORWARD;
			AttackNum = 9;
		}
		else if (inputs & INPUT_UP) {
			State |= STATE_UP;
			AttackNum = 10;
		}
		else if (inputs & INPUT_DOWN) {
			State |= STATE_DOWN;
			AttackNum = 11;
		}
		else {//Neutral
			AttackNum = 8;
		}
		if (State & STATE_JUMPING) {
			if ((inputs & INPUT_RIGHT && Facing.x == -1) || (inputs & INPUT_LEFT && Facing.x == 1)) {
				//Bair
				State |= STATE_BACK;
				AttackNum = 19;
			}
			else {
				AttackNum += 7;
			}
		}
		AttackDelay = frameNumber + Moves->GetMoveLength(AttackNum);
		++LastAttackID;
		AnimFrame = 0;
	}
	if (inputs & INPUT_HEAVY && !(State & (STATE_WEAK | STATE_HEAVY | STATE_SHIELDING | STATE_DROPSHIELD | STATE_JUMPSQUAT | STATE_JUMPLAND | STATE_HITSTUN | STATE_WAVEDASH | STATE_ATTACKSTUN | STATE_LEDGEHOG | STATE_KNOCKDOWN | STATE_KNOCKDOWNLAG))) {
		State &= ~STATE_CROUCH;
		if ((inputs & INPUT_RIGHT && Facing.x == 1) || (inputs & INPUT_LEFT && Facing.x == -1)) {
			//Fsmash
			State |= STATE_HEAVY | STATE_FORWARD;
			AttackNum = 12;
		}
		else if (inputs & INPUT_UP) {
			//Upsmash
			State |= STATE_HEAVY | STATE_UP;
			AttackNum = 13;
		}
		else if (inputs & INPUT_DOWN)
		{
			//Dsmash
			State |= STATE_HEAVY | STATE_DOWN;
			AttackNum = 14;
		}
		AttackDelay = frameNumber + Moves->GetMoveLength(AttackNum);
		++LastAttackID;
		AnimFrame = 0;
	}
	if (inputs & INPUT_JUMP && !(State & (STATE_WEAK | STATE_HEAVY | STATE_JUMPSQUAT | STATE_JUMPLAND | STATE_HITSTUN | STATE_SHIELDSTUN | STATE_WAVEDASH | STATE_ATTACKSTUN | STATE_KNOCKDOWNLAG))) {
		if (State & STATE_LEDGEHOG) {
			State &= ~STATE_LEDGEHOG;
			stage.Ledges[LedgeID].Hogged = false;
			State |= STATE_JUMPING | STATE_DOUBLEJUMPWAIT;
			JumpDelay = frameNumber + MaxJumpDelay;
			Velocity.y = -JumpSpeed;
			AnimFrame = 0;
		}
		else if (!(State & STATE_JUMPING)) {
			State |= STATE_JUMPSQUAT;
			JumpDelay = frameNumber + MaxJumpDelay;
			AnimFrame = 0;
			State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_SHIELDING | STATE_DROPSHIELD | STATE_RUNNING | STATE_CROUCH | STATE_KNOCKDOWN);
		}
		else if (HasDoubleJump) {
			HasDoubleJump = false;
			Velocity.y = -JumpSpeed;
			State &= ~STATE_FASTFALL;
		}
	}
	else if (!(inputs & INPUT_JUMP) && (State & STATE_JUMPSQUAT)) {
		State |= STATE_SHORTHOP;
	}
	if (inputs & INPUT_SHIELD) {
		if (inputs & INPUT_DOWN && State & STATE_JUMPSQUAT && HasAirDash) {
			HasAirDash = false;
			State &= ~(STATE_JUMPSQUAT | STATE_SHORTHOP);
			State |= STATE_WAVEDASH;
			JumpDelay = frameNumber + 2 * MaxJumpDelay;

			Velocity.y += DashSpeed;
			if (inputs & INPUT_LEFT) {
				Velocity.x -= DashSpeed;
			}
			else if (inputs & INPUT_RIGHT) {
				Velocity.x += DashSpeed;
			}
		}
		else if (State & STATE_HITSTUN && !(State & STATE_TECHLAG)) {
			State |= STATE_TECHATTEMPT;
			TechDelay = frameNumber + 20;
		}
		else if (State & STATE_JUMPING && !(State & (STATE_WEAK | STATE_HEAVY | STATE_HITSTUN)) && HasAirDash) {
			HasAirDash = false;
			State &= ~STATE_FASTFALL;
			if (inputs & INPUT_LEFT) {
				Velocity.x -= DashSpeed;
			}
			else if (inputs & INPUT_RIGHT) {
				Velocity.x += DashSpeed;
			}
			if (inputs & INPUT_UP) {
				Velocity.y -= DashSpeed;
			}
			else if (inputs & INPUT_DOWN) {
				Velocity.y += DashSpeed;
			}
		}
		else if (!(State & (STATE_WEAK | STATE_HEAVY | STATE_JUMPSQUAT | STATE_JUMPLAND | STATE_JUMPING | STATE_HITSTUN | STATE_SHIELDSTUN | STATE_WAVEDASH | STATE_ATTACKSTUN | STATE_LEDGEHOG | STATE_KNOCKDOWN | STATE_KNOCKDOWNLAG))) {
			State &= ~(STATE_DROPSHIELD | STATE_WAVEDASH | STATE_RUNNING);
			State |= STATE_SHIELDING;
			Velocity.x = 0;
		}
	}
	else if (State & STATE_SHIELDING) {
		State &= ~STATE_SHIELDING;
		State |= STATE_DROPSHIELD;
		AttackDelay += MaxShieldDelay;
	}

	//Update State
	if (State & (STATE_WEAK | STATE_HEAVY)) {
		Hitboxes = Moves->GetHitboxes(AttackNum, AnimFrame);
		Hurtboxes = Moves->GetHurtboxes(AttackNum, AnimFrame);
		numHitboxes = (int)(*Hitboxes).size();
	}
	else {
		Hitboxes = nullptr;
		numHitboxes = 0;
		if (State & STATE_HITSTUN) {
			Hurtboxes = Moves->GetHurtboxes(6, AnimFrame % 9);
		}
		else if (State & STATE_RUNNING) {
			Hurtboxes = Moves->GetHurtboxes(1, AnimFrame % 2);
		}
		else if (State & STATE_JUMPING) {
			Hurtboxes = Moves->GetHurtboxes(2, 0);
		}
		else if (State & STATE_JUMPSQUAT) {
			Hurtboxes = Moves->GetHurtboxes(3, AnimFrame);
		}
		else if (State & STATE_JUMPLAND) {
			Hurtboxes = Moves->GetHurtboxes(4, AnimFrame);
		}
		else if (State & (STATE_WAVEDASH | STATE_CROUCH)) {
			Hurtboxes = Moves->GetHurtboxes(7, 0);
		}
		else if (State & STATE_LEDGEHOG) {
			//Ledgehog anim
			Hurtboxes = Moves->GetHurtboxes(6, AnimFrame % 9); // Placeholder
		}
		else { //Idle
			Hurtboxes = Moves->GetHurtboxes(0,0);
		}
	}

	//Character Move-Specific Updates
	if ((State & STATE_HEAVY) && (State & STATE_UP) && AnimFrame == 17) {
		++LastAttackID;
	}
	else if ((State & STATE_JUMPING) && (State & STATE_WEAK) && (State & STATE_BACK) && AnimFrame == 7) {
		Facing.x *= -1;
	}

	//Adding Gravity
	if (State & STATE_JUMPING) {
		if (Velocity.y < MaxAirSpeed.y) {
			Velocity.y += Gravity;
		}
		if (State & STATE_FASTFALL) {
			Velocity.y += 3 * Gravity;
		}
	}

	//Adding Friction/Air resitance
	if (!(State & STATE_JUMPING)) {
		Velocity *= 0.9f;
	}
	else {
		if (fabs(Velocity.y) > MaxAirSpeed.y) {
			Velocity.y *= 0.8f;
		}
		if (fabs(Velocity.x) > MaxAirSpeed.x) {
			Velocity.x *= 0.95f;
		}
	}

	//Apply hits to other giraffes
	if (!(Hitboxes == nullptr)) {
		for (int j = 0; j < num_giraffes; ++j) {
			if (j != i) {
				for (int h = 0; h < numHitboxes; ++h) {
					(*giraffes[j]).AddHit((*Hitboxes)[h], LastAttackID, Facing, Position);
					
				}
			}
		}
	}
}

void NormGiraffe::Move(Stage& stage, const int frameNumber)
{
	//Recieve incoming hits
	if (State & STATE_SHIELDING) {
		for (int j = 0; j < numIncoming; ++j) {
			if (!PrevHitQueue.Contains(IncomingHits[j].ID)) {
				State |= STATE_SHIELDSTUN;
				AttackDelay = (int)max(AttackDelay, frameNumber + IncomingHits[j].hit.Damage * 50);
				PrevHitQueue.Push(IncomingHits[j].ID);
			}
		}
	}
	else {
		for (int j = 0; j < numIncoming; ++j) {
			if (!PrevHitQueue.Contains(IncomingHits[j].ID)) {
				Knockback += IncomingHits[j].hit.Damage;
				//Velocity += (Knockback / Mass) * IncomingHits[j].hit.Force;
				if (IncomingHits[j].hit.Fixed) {
					Velocity += IncomingHits[j].hit.Knockback * IncomingHits[j].hit.Force;
				}
				else {
					Velocity += ((((Knockback / 10 + (Knockback * IncomingHits[j].hit.Damage / 20)) * (200 / (Mass + 100)) * 1.4 + 0.18) * IncomingHits[j].hit.Scale) + IncomingHits[j].hit.Knockback) * IncomingHits[j].hit.Force;
				}
				if (State & STATE_LEDGEHOG) {
					stage.Ledges[LedgeID].Hogged = false;
					State &= ~STATE_LEDGEHOG;
				}
				State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_JUMPSQUAT | STATE_JUMPLAND | STATE_SHORTHOP | STATE_ATTACKSTUN | STATE_KNOCKDOWN | STATE_KNOCKDOWNLAG);
				State |= STATE_HITSTUN;
				AttackDelay = (int)max(AttackDelay, frameNumber + IncomingHits[j].hit.Knockback * 50);
				PrevHitQueue.Push(IncomingHits[j].ID);
			}
		}
	}
	numIncoming = 0;
	
	
	Position += Velocity;
	//Correct intersection with the stage
	bool landed = false;
	bool hogging = false;
	bool bounced = false;
	Vec2 offset;
	if (!(State & STATE_LEDGEHOG)) {
		if (stage.Intersects(Position, StageCollider, State & (STATE_CROUCH | STATE_FASTFALL), State & STATE_JUMPING, Velocity.y > -0.00001, landed, bounced, Facing, offset, Velocity, hogging, LedgeID)) {
			Position += offset;
			if (hogging) {
				State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_JUMPING | STATE_FASTFALL);
				State |= STATE_LEDGEHOG;
			}
			else if (bounced) {
				if (State & STATE_TECHATTEMPT) {
					Velocity = { 0,0 };
					State &= ~STATE_TECHATTEMPT;
				}
			}
			else if (landed && (State & STATE_JUMPING)) {
				if (State & STATE_HITSTUN && !(State & STATE_TECHATTEMPT)) {
					Velocity = { 0,0 };
					State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_JUMPING | STATE_FASTFALL | STATE_HITSTUN | STATE_TECHLAG);
					State |= STATE_KNOCKDOWNLAG;
					AnimFrame = 0;
					TechDelay = frameNumber + 30;
				}
				else {
					if (State & STATE_WEAK) {
						State |= STATE_ATTACKSTUN;
						AttackDelay = Moves->GetLandingLag(AttackNum - 15);
					}

					State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_JUMPING | STATE_FASTFALL | STATE_HITSTUN | STATE_TECHATTEMPT | STATE_TECHLAG);
					State |= STATE_JUMPLAND;
					HasAirDash = true;
					HasDoubleJump = false;
					JumpDelay = frameNumber + MaxJumpDelay / 2;
					AnimFrame = 0;
					TechDelay = 0;
				}
			}
		}
		else if (!(State & STATE_JUMPING)) {
			State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_SHIELDING | STATE_DROPSHIELD | STATE_SHIELDSTUN | STATE_JUMPSQUAT | STATE_SHORTHOP | STATE_JUMPLAND | STATE_WAVEDASH | STATE_ATTACKSTUN | STATE_CROUCH);
			State |= STATE_JUMPING | STATE_DOUBLEJUMPWAIT;
			JumpDelay = frameNumber + MaxJumpDelay * 2;
		}
	}


	//Make giraffes loop around like pac-man
	//Remove this later
	if (Position.y > 50) {
		Position.y = 0;
	}
	else if (Position.y < 0) {
		Position.y = 50;
	}
	if (Position.x > 60) {
		Position.x = 0;
	}
	else if (Position.x < 0) {
		Position.x = 60;
	}

	//This is the last stateful part of the update loop

}

void NormGiraffe::Draw(HDC hdc, Vec2 Scale, HBRUSH ShieldBrush)
{
	int CurrentAnim = 0;
	int CurrentFrame = 0;

	if (State & (STATE_WEAK | STATE_HEAVY)) {
		CurrentAnim = AttackNum;
		CurrentFrame = AnimFrame;
	}
	else {
		if (State & (STATE_HITSTUN | STATE_ATTACKSTUN)) {
			CurrentAnim = 6;
			CurrentFrame = AnimFrame % 9;
		}
		else if (State & STATE_SHIELDSTUN) {
			SelectObject(hdc, ShieldBrush);
			Ellipse(hdc, (Position.x - 2.5f) * Scale.x, (Position.y - 2.5f) * Scale.y, (Position.x + 2.5f) * Scale.x, (Position.y + 2.5f) * Scale.y);
			CurrentAnim = 6;
			CurrentFrame = AnimFrame % 9;
		}
		else if (State & STATE_SHIELDING) {
			SelectObject(hdc, ShieldBrush);
			Ellipse(hdc, (Position.x - 2.5f) * Scale.x, (Position.y - 2.5f) * Scale.y, (Position.x + 2.5f) * Scale.x, (Position.y + 2.5f) * Scale.y);
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
			CurrentFrame = AnimFrame % 9;
		}
		else if (State & (STATE_WAVEDASH | STATE_CROUCH)) {
			CurrentAnim = 7;
			CurrentFrame = 0;
		}
	}



	POINT points[NUM_POINTS];

	for (int i = 0; i < NUM_POINTS; ++i) {
		points[i] = (Scale * (Position + Facing * (*Moves->GetSkelPoints(CurrentAnim, CurrentFrame))[i])).ToPoint();
	}

	//Ellipse(hdc, (int)(Scale.x*(Position.x - 2.5)), (int)(Scale.y * (Position.y - 2.5)), (int)(Scale.x * (Position.x + 2.5)), (int)(Scale.y * (Position.y + 2.5)));
	//Polyline(hdc, points, NUM_POINTS);
	Polyline(hdc, points, 27);
	PolyBezier(hdc, &points[26], 4);
	Polyline(hdc, &points[29], 5);
	PolyBezier(hdc, &points[33], 4);
	Polyline(hdc, &points[36], 2);
}
