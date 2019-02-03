//	APU - SR-71r Orbiter Addon
//	Copyright(C) 2015  Blake Christensen
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

#include "bc_orbiter\BaseVessel.h"
#include "bc_orbiter\PoweredComponent.h"
#include "bc_orbiter\Animation.h"
#include "bc_orbiter\OnOffSwitch.h"
#include "bc_orbiter\IAnimationState.h"
#include "bc_orbiter\VCToggleSwitch.h"
#include "bc_orbiter\VCGauge.h"

#include "PropulsionController.h"
#include "SR71r_mesh.h"

// TEMP SPECS:

const double APU_BURN_RATE = 0.05;   // kg per second - 180 kg per hour (180 / 60) / 60.
const double APU_MIN_VOLT = 20.0;
class VESSEL3;

/**	APU
	Auxiliary Power Unit.  The APU provide power to the hydraulic system of the aircraft
	which in turn powers the flight control surfaces and landing gear.	The APU draws 
    from the same fuel source as the RCS system.
	
    When running, the 'APU' light will be illuminated on the status board.

	Hydraulic level is currently all or nothing. All = 1.0, Nothing = 0.0;

	The APU requires electrical power to function.
	
	Configuration:
	APU = 0/1

	Short cuts:
	None.
*/
class APU : 
	public bco::PoweredComponent,
	public bco::IAnimationState
{
public:
	APU(bco::BaseVessel* vessel, double amps);

	// *** PoweredComponent ***
	/**
		Draws a fixed amp load when the main circuit is powered and
		the APU switch is on.
	*/
	virtual double CurrentDraw() override;

	// *** Component ***
    virtual void SetClassCaps() override;
    virtual bool VCRedrawEvent(int id, int event, SURFHANDLE surf) override { return false; }
	virtual bool LoadConfiguration(char* key, FILEHANDLE scn, const char* configLine) override;
	virtual void SaveConfiguration(FILEHANDLE scn) const override;
    
    // *** IAnimationState ***
    virtual double GetState() const override { return GetHydraulicLevel(); }

    void Step(double simt, double simdt, double mjd);

    // *** APU ***
	bco::OnOffSwitch& PowerSwitch();
	double GetHydraulicLevel() const;
	void SetPropulsionControl(PropulsionController* pc);

private:
	PropulsionController*	propulsionControl_;

	const char*				ConfigKey = "APU";

	bool					isSoundStopped = false;

    bco::VCGauge            gaugeHydrPress_{ {bt_mesh::SR71rVC::gaHydPress_id },
                                                bt_mesh::SR71rVC::gaHydPress_location, bt_mesh::SR71rVC::axisHydPress_location,
                                                (300 * RAD),
                                                0.2
                                            };

    bco::VCToggleSwitch     swPower_        {   bt_mesh::SR71rVC::SwAPUPower_id,
											    GetBaseVessel(),
                                                bt_mesh::SR71rVC::SwAPUPower_location,
                                                bt_mesh::SR71rVC::LeftPanelTopRightAxis_location
                                            };

	virtual void SetSound();

	virtual void StopAllSounds();
};