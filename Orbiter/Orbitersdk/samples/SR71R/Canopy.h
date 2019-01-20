//	Canopy - SR-71r Orbiter Addon
//	Copyright(C) 2017  Blake Christensen
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

#pragma once

#include "OrbiterSDK.h"

#include "bc_orbiter\PoweredComponent.h"
#include "bc_orbiter\OnOffSwitch.h"
#include "bc_orbiter\Animation.h"
#include "bc_orbiter\VCToggleSwitch.h"

#include "SR71r_mesh.h"

namespace bco = bc_orbiter;

/**	Canopy
Controls the canopy door.
The canopy runs on the main electrical circuit and can have a high draw
when in motion.

To operate the canopy:
- Power the main circuit (external or fuel cell).
- Turn 'Main' power on (up).
- Turn 'Canopy' (CNPY) power switch (right panel) on (up).
- To Open switch 'Canopy' switch (left panel) to OPEN.
- The message board [CNPY] light will show orange while the canopy is
in motion, and white when fully open.

Configuration:

CANOPY a b c
a - 0/1 Power switch off/on.
b - 0/1 Open close switch closed/open.
c - 0.0-1.0 Current canopy position.

*/
class Canopy : public bco::PoweredComponent
{
public:
    Canopy(bco::BaseVessel* vessel, double amps);

    virtual void SetClassCaps() override;
	virtual bool VCRedrawEvent(int id, int event, SURFHANDLE surf) override { return false; }
	virtual bool LoadConfiguration(char* key, FILEHANDLE scn, const char* configLine) override;
	virtual void SaveConfiguration(FILEHANDLE scn) const override;

    /**
    The draw is only active when in motion.
    */
    virtual double CurrentDraw() override;

    /**
    Provide time steps for the animations.
    */
    void Step(double simt, double simdt, double mjd);


    bco::OnOffSwitch&	CanopyPowerSwitch();
    bco::OnOffSwitch&	CanopyOpenSwitch();

    double				GetCanopyState();

private:
    bool CanopyHasPower();

    const char*			    ConfigKeyCanopy = "CANOPY";

    bco::Animation		    animCanopy_     {   &swCanopyOpen_, 0.2};
    UINT                    idAnim_         { 0 };

    bco::VCToggleSwitch     swCanopyPower_  {   bt_mesh::SR71rVC::SwCanopyPower_id,
                                                bt_mesh::SR71rVC::SwCanopyPower_location,
                                                bt_mesh::SR71rVC::PowerTopRightAxis_location
                                            };

    bco::VCToggleSwitch     swCanopyOpen_   {   bt_mesh::SR71rVC::SwCanopyOpen_id,
                                                bt_mesh::SR71rVC::SwCanopyOpen_location,
                                                bt_mesh::SR71rVC::DoorsRightAxis_location
                                            };

    bco::AnimationGroup     gpCanopy_       { { bt_mesh::SR71r::CanopyFO_id,
                                                bt_mesh::SR71r::ForwardCanopyWindow_id,
                                                bt_mesh::SR71r::CanopyFI_id,
												bt_mesh::SR71r::CanopyWindowSI_id,
												bt_mesh::SR71r::CanopyWindowInsideLeft_id},
                                                bt_mesh::SR71r::CockpitAxisS_location, bt_mesh::SR71r::CockpitAxisP_location,
                                                (55 * RAD),
                                                0, 1
                                            };

    bco::AnimationGroup     gpCanopyVC_     { { bt_mesh::SR71rVC::CanopyFI_id,
                                                bt_mesh::SR71rVC::CanopyFO_id,
                                                bt_mesh::SR71rVC::CanopyWindowInsideLeft_id,
                                                bt_mesh::SR71rVC::CanopyWindowSI_id },
                                                bt_mesh::SR71r::CockpitAxisS_location, bt_mesh::SR71r::CockpitAxisP_location,
                                                (55 * RAD),
                                                0, 1
                                            };
};