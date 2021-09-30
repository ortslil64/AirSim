#include "ProbotPawnApi.h"
#include "AirBlueprintLib.h"

#include "PhysXVehicleManager.h"
#include "ProbotPawn.h"
#include "CarPawn.h"

ProbotPawnApi::ProbotPawnApi(ACarPawn* pawn, const msr::airlib::Kinematics::State* pawn_kinematics,
                        msr::airlib::CarApiBase* vehicle_api)
    : CarPawnApi(pawn,pawn_kinematics,vehicle_api)
{
    movement_ = pawn->GetVehicleMovement();
    pawn_ = static_cast<AProbotPawn*>(pawn);
}

void ProbotPawnApi::updateMovement(const msr::airlib::CarApiBase::CarControls& controls)
{
    last_controls_ = controls;

    MotionControlOutput controlOutput;
    controlOutput.validFields = static_cast<EPossibleCommands>(EPC_STEERING | EPC_THROTTLE);
    controlOutput.throttleCommand = FGenericPlatformMath::Fmod(controls.throttle * 100, 60);
    controlOutput.steeringCommand = FGenericPlatformMath::Fmod(-controls.steering * 2 * 100, 60);
    pawn_->m_pMotionModel->SetControlCommands(controlOutput);
    
/*
    if (!controls.is_manual_gear && movement_->GetTargetGear() < 0)
        movement_->SetTargetGear(0, true); //in auto gear we must have gear >= 0
    if (controls.is_manual_gear && movement_->GetTargetGear() != controls.manual_gear)
        movement_->SetTargetGear(controls.manual_gear, controls.gear_immediate);

    movement_->SetThrottleInput(controls.throttle);
    movement_->SetSteeringInput(controls.steering);
    movement_->SetBrakeInput(controls.brake);
    movement_->SetHandbrakeInput(controls.handbrake);
    movement_->SetUseAutoGears(!controls.is_manual_gear);*/
}

msr::airlib::CarApiBase::CarState ProbotPawnApi::getCarState() const
{
        //     msr::airlib::CarApiBase::CarState state(
//         movement_->GetForwardSpeed() / 100, //cm/s -> m/s
//         movement_->GetCurrentGear(),
//         movement_->GetEngineRotationSpeed(),
//         movement_->GetEngineMaxRotationSpeed(),
//         last_controls_.handbrake,
//         *pawn_kinematics_,
//         msr::airlib::ClockFactory::get()->nowNanos());
    return msr::airlib::CarApiBase::CarState();
}

/*
void ProbotPawnApi::reset()
{
    vehicle_api_->reset();

    last_controls_ = msr::airlib::CarApiBase::CarControls();
    auto phys_comps = UAirBlueprintLib::getPhysicsComponents(pawn_);
    UAirBlueprintLib::RunCommandOnGameThread([this, &phys_comps]() {
        for (auto* phys_comp : phys_comps) {
            phys_comp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
            phys_comp->SetPhysicsLinearVelocity(FVector::ZeroVector);
            phys_comp->SetSimulatePhysics(false);
        }
        movement_->ResetMoveState();
        movement_->SetActive(false);
        movement_->SetActive(true, true);
        vehicle_api_->setCarControls(msr::airlib::CarApiBase::CarControls());
        updateMovement(msr::airlib::CarApiBase::CarControls());

        auto pv = movement_->PVehicle;
        if (pv) {
            pv->mWheelsDynData.setToRestState();
        }
        auto pvd = movement_->PVehicleDrive;
        if (pvd) {
            pvd->mDriveDynData.setToRestState();
        }
    },
                                             true);

    UAirBlueprintLib::RunCommandOnGameThread([this, &phys_comps]() {
        for (auto* phys_comp : phys_comps)
            phys_comp->SetSimulatePhysics(true);
    },
                                             true);
}

void ProbotPawnApi::update()
{
    vehicle_api_->updateCarState(getCarState());
    vehicle_api_->update();
}*/

ProbotPawnApi::~ProbotPawnApi() = default;
