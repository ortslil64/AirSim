#pragma once

#include "vehicles/car/api/CarApiBase.hpp"
#include "WheeledVehicleMovementComponent4W.h"
#include "physics/Kinematics.hpp"
#include "CarPawn.h"

class CarPawnApi
{
public:
    typedef msr::airlib::ImageCaptureBase ImageCaptureBase;

   CarPawnApi(ACarPawn* pawn, const msr::airlib::Kinematics::State* pawn_kinematics,
               msr::airlib::CarApiBase* vehicle_api);

    virtual void updateMovement(const msr::airlib::CarApiBase::CarControls& controls);

    virtual msr::airlib::CarApiBase::CarState getCarState() const;

    virtual void reset();
    virtual void update();

    virtual ~CarPawnApi();

protected:
    msr::airlib::CarApiBase::CarControls last_controls_;
    ACarPawn* pawn_;
    const msr::airlib::Kinematics::State* pawn_kinematics_;
    msr::airlib::CarApiBase* vehicle_api_;

private:
    UWheeledVehicleMovementComponent* movement_;
};
