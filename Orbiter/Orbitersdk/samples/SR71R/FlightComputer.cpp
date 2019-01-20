//	FlightComputer - SR-71r Orbiter Addon
//	Copyright(C) 2016  Blake Christensen
//
//	This program is free software : you can redistribute it and / or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.If not, see <http://www.gnu.org/licenses/>.

#include "stdafx.h"
#include "FlightComputer.h"
#include "SR71r_mesh.h"

#include <assert.h>

namespace mvc = bt_mesh::SR71rVC;

FC::FlightComputer::FlightComputer(bco::BaseVessel * vessel, double amps) :
	bco::PoweredComponent(vessel, 10.0, 25.0),
	visAPMainOn_(		bt_mesh::SR71rVC::SwAPMain_verts,		bt_mesh::SR71rVC::SwAPMain_id),
	visAPHeadingOn_(	bt_mesh::SR71rVC::SwAPHeading_verts,	bt_mesh::SR71rVC::SwAPHeading_id),
	visAPAltitudeOn_(	bt_mesh::SR71rVC::SwAPAltitude_verts,	bt_mesh::SR71rVC::SwAPAltitude_id),
	visAPKEASOn_(		bt_mesh::SR71rVC::SwAPKEAS_verts,		bt_mesh::SR71rVC::SwAPKEAS_id),
	visAPMACHOn_(		bt_mesh::SR71rVC::SwAPMACH_verts,		bt_mesh::SR71rVC::SwAPMACH_id),
	prgHoldAltitude_(	*this),
	prgHoldHeading_(	*this),
	prgHoldKeas_(		*this),
	prgHoldMach_(		*this)
{
	swAPMain_.SetPressedFunc([this] 
	{
		ToggleAtmoProgram(FCProgFlags::AtmoActive); 
		isDisplayDirty_ = true;
	});
	vessel->RegisterVCEventTarget(&swAPMain_);

	swAPHeading_.SetPressedFunc([this] 
	{
		ToggleAtmoProgram(FCProgFlags::HoldHeading); 
		isDisplayDirty_ = true;
	});
	vessel->RegisterVCEventTarget(&swAPHeading_);

	swAPHAltitude_.SetPressedFunc([this]
	{
		ToggleAtmoProgram(FCProgFlags::HoldAltitude);
		isDisplayDirty_ = true;
	});
	vessel->RegisterVCEventTarget(&swAPHAltitude_);

	swAPKEAS_.SetPressedFunc([this]
	{
		ToggleAtmoProgram(FCProgFlags::HoldKEAS);
		isDisplayDirty_ = true;
	});
	vessel->RegisterVCEventTarget(&swAPKEAS_);

	swAPMACH_.SetPressedFunc([this]
	{
		ToggleAtmoProgram(FCProgFlags::HoldMACH);
		isDisplayDirty_ = true;
	});
	vessel->RegisterVCEventTarget(&swAPMACH_);
}

void FC::FlightComputer::SetClassCaps()
{
}

bool FC::FlightComputer::DrawHUD(int mode, const HUDPAINTSPEC* hps, oapi::Sketchpad* skp)
{
	if (oapiCockpitMode() != COCKPIT_VIRTUAL) return false;

	if (IsProgramRunning(FCProgFlags::AtmoActive))
	{
		int xLeft = hps->W - 1;
		int yTop = hps->H - 30;

		skp->Rectangle(xLeft - 30,
			yTop,
			xLeft,
			yTop + 25);

		skp->Text(xLeft - 27, yTop + 1, "AP", 2);

		skp->Line(xLeft - 77, yTop, xLeft - 77, yTop + 25);
		skp->Line(xLeft - 127, yTop, xLeft - 127, yTop + 25);

		if (IsProgramRunning(FCProgFlags::HoldHeading))
		{
			skp->Text(xLeft - 72, yTop + 1, "HDG", 3);
		}

		if (IsProgramRunning(FCProgFlags::HoldAltitude))
		{
			skp->Text(xLeft - 122, yTop + 1, "ALT", 3);
		}

		if (IsProgramRunning(FCProgFlags::HoldKEAS))
		{
			skp->Text(xLeft - 162, yTop + 1, "KS", 2);
		}
		
		if (IsProgramRunning(FCProgFlags::HoldMACH))
		{
			skp->Text(xLeft - 162, yTop + 1, "MA", 2);
		}
	}
	return true;
}


void FC::FlightComputer::Boot()
{
	if (!HasPower()) return;

	MapKey(GCKey::Clear,		[this] { HandleScratchPadKey(GCKey::Clear); });
	MapKey(GCKey::D0,			[this] { HandleScratchPadKey(GCKey::D0); });
	MapKey(GCKey::D1,			[this] { HandleScratchPadKey(GCKey::D1); });
	MapKey(GCKey::D2,			[this] { HandleScratchPadKey(GCKey::D2); });
	MapKey(GCKey::D3,			[this] { HandleScratchPadKey(GCKey::D3); });
	MapKey(GCKey::D4,			[this] { HandleScratchPadKey(GCKey::D4); });
	MapKey(GCKey::D5,			[this] { HandleScratchPadKey(GCKey::D5); });
	MapKey(GCKey::D6,			[this] { HandleScratchPadKey(GCKey::D6); });
	MapKey(GCKey::D7,			[this] { HandleScratchPadKey(GCKey::D7); });
	MapKey(GCKey::D8,			[this] { HandleScratchPadKey(GCKey::D8); });
	MapKey(GCKey::D9,			[this] { HandleScratchPadKey(GCKey::D9); });
	MapKey(GCKey::Decimal,		[this] { HandleScratchPadKey(GCKey::Decimal); });
	MapKey(GCKey::PlusMinus,	[this] { HandleScratchPadKey(GCKey::PlusMinus); });

	MapKey(GCKey::Home,			[this] { PageMain(); });
	


	PageMain();


	//ClearScreen();

	//DisplayLine(0, "ASCENT          1/1");
	//AscentSetAltTarget(0.0);
	//AscentSetIncTarget(0.0);

	//SetScratchPad(92.3);
	isRunning_ = true;
}


void FC::FlightComputer::ChangePowerLevel(double newLevel)
{
    PoweredComponent::ChangePowerLevel(newLevel);

	if (!HasPower())
	{
		isRunning_ = false;
		ClearScreen();
	}
}

void FC::FlightComputer::Step(double simt, double simdt, double mjd)
{
	// Handle low lever computer stuff, boot etc.
	Update();

	if (!isRunning_) return;

    // Handle key input
    if (!keyBuffer_.empty())
    {
        auto key = keyBuffer_.back();
        keyBuffer_.pop_back();
		mapKeyFunc_[key]();
    }

	// Give control programs simulator time.
	//vesselCtrl_.Step(simt, simdt, mjd);
	if (prevRunningProgs != runningPrograms_) UpdateProgs(runningPrograms_);

	auto atmoOn = IsProgramRunning(FCProgFlags::AtmoActive);

	if (atmoOn && IsProgramRunning(FCProgFlags::HoldAltitude))	prgHoldAltitude_.Step(simt, simdt, mjd);
	if (atmoOn && IsProgramRunning(FCProgFlags::HoldHeading))	prgHoldHeading_.Step(simt, simdt, mjd);
	if (atmoOn && IsProgramRunning(FCProgFlags::HoldKEAS))		prgHoldKeas_.Step(simt, simdt, mjd);
	if (atmoOn && IsProgramRunning(FCProgFlags::HoldMACH))		prgHoldMach_.Step(simt, simdt, mjd);

	prevRunningProgs = runningPrograms_;



	// Now call the update method for the active page.
	if (fabs(simt - prevTime_) > 0.2)
	{
		funcUpdate_();
		prevTime_ = simt;
	}
}

double FC::FlightComputer::CurrentDraw()
{
	// TODO: Implement power draw based on running programs.
	return 0.0;
}

void FC::FlightComputer::ClearScreen()
{
	for (int i = 0; i < DISPLAY_ROWS; i++)
	{
		DisplayLine(i, "                   ");
	}
}

void FC::FlightComputer::DisplayLine(int row, char* mask, ...)
{
	va_list argptr;
	va_start(argptr, mask);
	vsnprintf(display_[row], DISPLAY_COLS, mask, argptr);
	va_end(argptr);

	isDisplayDirty_ = true;
}

void FC::FlightComputer::DisplayText(int row, int col, const char* text)
{
	if (row < 0 || row > DISPLAY_ROWS - 1) return;
	if (col < 0 || col > DISPLAY_COLS - 1) return;

	int offs = col;
	size_t tIdx = 0;
	auto textLen = strnlen(text, DISPLAY_COLS);

	while (offs < DISPLAY_COLS && tIdx < textLen)
	{
		if (text[tIdx] != '~')		// use ~ to mask parts of a string not to update.
		{
			display_[row][offs] = text[tIdx];
		}

		tIdx++;
		offs++;
	}

	isDisplayDirty_ = true;
}

double FC::FlightComputer::GetScratchPad()
{
	auto temp = scratchValue_;
	ClearScratchPad();
	return temp;
}

void FC::FlightComputer::SetScratchPad(double value)
{
	scratchValue_ = value;

	DisplayLine(10, "[%10.2f]", scratchValue_);
}

bool FC::FlightComputer::MouseEvent(int id, int event)
{
	auto keypressed = mapKey_.find(id);
    if (keypressed != mapKey_.end())
    {
        keyBuffer_.push_back(keypressed->second);
    }
	
	return false;
}

bool FC::FlightComputer::LoadVC(int id)
{
    auto vcMeshHandle = GetBaseVessel()->GetVCMeshHandle0();
    assert(vcMeshHandle != nullptr);

    SURFHANDLE surfHandle = oapiGetTextureHandle(vcMeshHandle, bt_mesh::SR71rVC::TXIDX_SR71R_100_VC2_dds);

    // Init our font:
    vcFont_.surfSource = surfHandle;
    vcFont_.charWidth = 12;
    vcFont_.charHeight = 20;
    vcFont_.sourceX = 4;
    vcFont_.sourceY = 2021;
    vcFont_.blankX = 1600;
    vcFont_.blankY = 2021;

    auto eventId = GetBaseVessel()->RegisterVCEvent(this, bco::VCIdMode::All);
    oapiVCRegisterArea(
        eventId,
        _R(80, 835, 320, 1060), //_R(1710, 95, 1950, 320),
        PANEL_REDRAW_USER,
        PANEL_MOUSE_LBDOWN | PANEL_MOUSE_LBPRESSED | PANEL_MOUSE_ONREPLAY,
        PANEL_MAP_BACKGROUND,
        surfHandle);

    allId_ = eventId;

    // mapKeyLocation_ maps a GCKey to the mouse click location on the mesh.
    // We walk through the map here to register the mouse events and setup the
    // mouse event to GCKey map.
    for (auto& k : mapKeyLocation_)
    {
        auto eid = GetBaseVessel()->RegisterVCMouseEvent(this);
        oapiVCRegisterArea(eid, PANEL_REDRAW_NEVER, PANEL_MOUSE_DOWN);
        oapiVCSetAreaClickmode_Spherical(eid, k.second, 0.01);
        mapKey_[eid] = k.first;
    }

	//allId_ = GetBaseVessel()->RegisterVCEvent(this, bco::VCIdMode::RedrawEvent);
	//oapiVCRegisterArea(allId_, PANEL_REDRAW_USER, PANEL_MOUSE_IGNORE);


    return true;
}

bool FC::FlightComputer::VCRedrawEvent(int id, int event, SURFHANDLE surf)
{
	auto devMesh = GetBaseVessel()->GetVirtualCockpitMesh0();
	assert(devMesh != nullptr);

	for (int i = 0; i < DISPLAY_ROWS; i++)
	{
		bco::DrawSurfaceText(0, i * 20, display_[i], bco::DrawTextFormat::Left, surf, vcFont_);
	}

	const double offset = 0.0352;
	double trans = 0.0;

	trans = IsProgramRunning(FCProgFlags::AtmoActive) ? offset : 0.0;
	visAPMainOn_.SetTranslate(_V(trans, 0.0, 0.0));
	visAPMainOn_.Draw(devMesh);

	trans = IsProgramRunning(FCProgFlags::HoldHeading) ? offset : 0.0;
	visAPHeadingOn_.SetTranslate(_V(trans, 0.0, 0.0));
	visAPHeadingOn_.Draw(devMesh);

	trans = IsProgramRunning(FCProgFlags::HoldAltitude) ? offset : 0.0;
	visAPAltitudeOn_.SetTranslate(_V(trans, 0.0, 0.0));
	visAPAltitudeOn_.Draw(devMesh);

	trans = IsProgramRunning(FCProgFlags::HoldKEAS) ? offset : 0.0;
	visAPKEASOn_.SetTranslate(_V(trans, 0.0, 0.0));
	visAPKEASOn_.Draw(devMesh);

	trans = IsProgramRunning(FCProgFlags::HoldMACH) ? offset : 0.0;
	visAPMACHOn_.SetTranslate(_V(trans, 0.0, 0.0));
	visAPMACHOn_.Draw(devMesh);

	return true;
}

bool FC::FlightComputer::LoadConfiguration(char * key, FILEHANDLE scn, const char * configLine)
{
	if (_strnicmp(key, apConfigKey_.c_str(), apConfigKey_.length()) == 0)
	{
		return LoadAPConfiguration(scn, configLine);
	}

	return false;
}

bool FC::FlightComputer::LoadAPConfiguration(FILEHANDLE scn, const char* configLine)
{
	int isOn;
	int isHoldHeading;
	int isHoldAltitude;
	int isHoldSpeed;
	int isHoldMach;

	sscanf_s(configLine + apConfigKey_.length(), "%d%d%d%d%d", &isOn, &isHoldHeading, &isHoldAltitude, &isHoldSpeed, &isHoldMach);

	SetProgramState(FCProgFlags::AtmoActive,	(isOn == 1));
	SetProgramState(FCProgFlags::HoldHeading,	(isHoldHeading == 1));
	SetProgramState(FCProgFlags::HoldAltitude,	(isHoldAltitude == 1));
	
	if (isHoldSpeed == 1)
	{
		auto spType = (isHoldMach == 1) ? FCProgFlags::HoldMACH : FCProgFlags::HoldKEAS;
		SetProgramState(spType, true);
	}

	return true;
}

void FC::FlightComputer::SaveConfiguration(FILEHANDLE scn) const
{
	SaveAPConfiguration(scn);
}

void FC::FlightComputer::SaveAPConfiguration(FILEHANDLE scn) const
{
	char cbuf[256];

	auto pwr = (IsProgramRunning(FCProgFlags::AtmoActive)) ? 1 : 0;
	auto hd = (IsProgramRunning(FCProgFlags::HoldHeading)) ? 1 : 0;
	auto al = (IsProgramRunning(FCProgFlags::HoldAltitude)) ? 1 : 0;
	auto sp = (IsProgramRunning(FCProgFlags::HoldKEAS) || IsProgramRunning(FCProgFlags::HoldMACH)) ? 1 : 0;
	auto km = (IsProgramRunning(FCProgFlags::HoldMACH)) ? 1 : 0;

	sprintf_s(cbuf, "%d %d %d %d %d", pwr, hd, al, sp, km);

	oapiWriteScenario_string(scn, (char*)apConfigKey_.c_str(), cbuf);
}

void FC::FlightComputer::PageMain()
{
	ClearScreen();
	DisplayLine(0, "MAIN         PG 1:1");
	DisplayLine(1, "<ASCENT");
	DisplayLine(3, "<ORBIT OPS");
	DisplayLine(5, "<REENTRY");
	DisplayLine(7, "<AUTO PILOT");

	ClearFuncKeys();
	MapKey(GCKey::F1, [this] { PageAscent(); });
	MapKey(GCKey::F2, [this] { PageOrbitOps(); });
	MapKey(GCKey::F3, [this] { PageReEntry(); });
	MapKey(GCKey::F4, [this] { PageAtmosphere(); });

	funcUpdate_ = [] {};
}

void FC::FlightComputer::PageAtmosphere()
{
	ClearScreen();
	DisplayLine(0, "AUTO PILOT   PG 1:1");

	ClearFuncKeys();
	MapKey(GCKey::F1,	[this] { ToggleAtmoProgram(FCProgFlags::HoldHeading); });
	MapKey(GCKey::F2,	[this] { ToggleAtmoProgram(FCProgFlags::HoldAltitude); });
	MapKey(GCKey::F3,	[this] { ToggleAtmoProgram(FCProgFlags::HoldKEAS); });
	MapKey(GCKey::F4,	[this] { ToggleAtmoProgram(FCProgFlags::HoldMACH); });
	MapKey(GCKey::F10,	[this] { ToggleAtmoProgram(FCProgFlags::AtmoActive); });


	prevRunning_ = FCProgFlags::HoldInvalid;  // Force re-eval
	funcUpdate_ = [this] { UpdateAtmospherePage(); };
}

void FC::FlightComputer::UpdateAtmospherePage()
{
	if (runningPrograms_ != prevRunning_)
	{
		DisplayLine(1, "<HDG   -%s-", IsProgramRunning(FCProgFlags::HoldHeading) ? "ON " : " . ");
		DisplayLine(3, "<ALT   -%s-", IsProgramRunning(FCProgFlags::HoldAltitude) ? "ON " : " . ");
		DisplayLine(5, "<KEAS  -%s-", IsProgramRunning(FCProgFlags::HoldKEAS) ? "ON " : " . ");
		DisplayLine(7, "<MACH  -%s-", IsProgramRunning(FCProgFlags::HoldMACH) ? "ON " : " . ");
		DisplayLine(9, " -%s-   -%s->", 
			IsProgramRunning(FCProgFlags::AtmoActive) ? "RUNNING" : "   .   ",
			IsProgramRunning(FCProgFlags::AtmoActive) ? "STOP>" : " RUN>");
		
		prevRunning_ = runningPrograms_;
	}
}

void FC::FlightComputer::PageAscent()
{
	ClearScreen();
	DisplayLine(0, "ASCENT       PG 1:1");

	ClearFuncKeys();
	MapKey(GCKey::F1, [this] { SetTargetIncDeg(GetScratchPad()); });

	MapKey(GCKey::F7, [this] { GetAvionics()->SetHeading(ascentHeading_); });
	MapKey(GCKey::F8, [this] { GetAvionics()->SetHeading(ascentHeadingAlt_); });

	// Target cannot be less than lat.
	double vlng, vlat, vrad, slope;
	OBJHANDLE hRef = GetBaseVessel()->GetEquPos(vlng, vlat, vrad);

	launchLatitude_ = vlat;
	SetTargetIncDeg(DEG * launchLatitude_);

	funcUpdate_ = [this] { UpdateAscentPage(); };
}

void FC::FlightComputer::UpdateAscentPage()
{
	if (!isAscentPageDirty_) return;

	DisplayLine(1, "<TGT INC  :%6.2f", DEG * ascentTargetInc_);
	DisplayLine(3, "       SET HDG %3.0f>", DEG * ascentHeading_);
	DisplayLine(5, "       SET HDG %3.0f>", DEG * ascentHeadingAlt_);

	isAscentPageDirty_ = false;
}

/**	SetTargetIncDeg
	@param tgt Target Inclination in degrees.
	Takes the target inclincation and sets the ascentHeading, both
	direct and alternate. (-180).  Inclination must be valid for 
	the current launchLatitude.
*/
void FC::FlightComputer::SetTargetIncDeg(double tgt)
{
	if (bco::CalcLaunchHeading(launchLatitude_, RAD * tgt, ascentHeading_, ascentHeadingAlt_))
	{
		ascentTargetInc_ = tgt * RAD;
	}
	else
	{
		SetScratchError("Invalid Input");
	}

	isAscentPageDirty_ = true;
}

/**
	Boots the computer if needed, checks for dirty display.
	Does not update in program states.
*/
void FC::FlightComputer::Update()
{
	if (!isRunning_)
	{
		if (HasPower())
		{
			Boot();
		}
	}

    if (isDisplayDirty_)
    {
        oapiVCTriggerRedrawArea(0, allId_);
        isDisplayDirty_ = false;
    }
}

void FC::FlightComputer::ClearFuncKeys()
{
	MapKey(GCKey::F1);
	MapKey(GCKey::F2);
	MapKey(GCKey::F3);
	MapKey(GCKey::F4);
	MapKey(GCKey::F5);
	MapKey(GCKey::F6);
	MapKey(GCKey::F7);
	MapKey(GCKey::F8);
	MapKey(GCKey::F9);
	MapKey(GCKey::F10);
}

void FC::FlightComputer::ClearScratchPad()
{
	scratchKeys_.clear();
	scratchIsPos_ = true;
	SetScratchPad(0.0);
}

void FC::FlightComputer::SetScratchError(const char* msg)
{
	isScratchError_ = true;
	DisplayLine(10, "!! %s", msg);
}

bool FC::FlightComputer::HandleScratchPadKey(FC::GCKey key)
{
	if (key == FC::GCKey::Clear)
	{
		ClearScratchPad();
		isScratchError_ = false;
		return true;
	}

	if (isScratchError_) return false;


	auto evaluate = false;
	double newValue = 0.0;

	switch (key)
	{
	case FC::GCKey::D0:
	case FC::GCKey::D1:
	case FC::GCKey::D2:
	case FC::GCKey::D3:
	case FC::GCKey::D4:
	case FC::GCKey::D5:
	case FC::GCKey::D6:
	case FC::GCKey::D7:
	case FC::GCKey::D8:
	case FC::GCKey::D9:
		if (scratchKeys_.size() < 8)
		{
			scratchKeys_.append(std::to_string(key));
			evaluate = true;
		}
		break;

	case FC::GCKey::Decimal:
		if (scratchKeys_.find('.') == std::string::npos)
		{
			scratchKeys_.append(".");
			evaluate = true;
		}
		break;

	case FC::GCKey::PlusMinus:
		scratchIsPos_ = !scratchIsPos_;
		evaluate = true;
		break;
	}

	if (evaluate)
	{
		newValue = atof(scratchKeys_.c_str());
		if (!scratchIsPos_) newValue *= -1.0;

		SetScratchPad(newValue);
	}

	return true;
}

