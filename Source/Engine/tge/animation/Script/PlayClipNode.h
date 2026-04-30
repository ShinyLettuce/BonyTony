#include <tge/animation/PoseGenerator.h>
#include <tge/script/ScriptCommon.h>
#include <tge/script/ScriptNodeBase.h>

namespace Tga
{
	class PlayClipNode : public ScriptNodeBase
	{
		ScriptPinId myPoseOutPin;
		ScriptPinId myAnimationClipInPin;

	public:
		void Init(const ScriptCreationContext& context) override;
		virtual std::unique_ptr<ScriptNodeRuntimeInstanceBase> CreateRuntimeInstanceData() const override;

		Property ReadPin(ScriptExecutionContext& context, ScriptPinId) const override;
	};
}