#include <tge/animation/Pose.h>
#include <tge/animation/Skeleton.h>
#include <tge/script/ScriptCommon.h>
#include <tge/script/ScriptNodeBase.h>

namespace Tga
{
	class BlendPoseNode : public ScriptNodeBase
	{
		ScriptPinId myPoseAInPin;
		ScriptPinId myPoseBInPin;
		ScriptPinId myBlendAmountPin;

		ScriptPinId myPoseOutPin;

	public:
		void Init(const ScriptCreationContext& context) override;
		virtual std::unique_ptr<ScriptNodeRuntimeInstanceBase> CreateRuntimeInstanceData() const override;
		Property ReadPin(ScriptExecutionContext& context, ScriptPinId) const override;
	};
}