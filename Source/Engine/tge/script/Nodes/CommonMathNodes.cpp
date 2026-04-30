#include <stdafx.h>
#include "CommonMathNodes.h"

#include <tge/script/ScriptNodeTypeRegistry.h>
#include <tge/stringRegistry/StringRegistry.h>
#include <tge/script/ScriptCommon.h>
#include <tge/script/ScriptNodeBase.h>
#include <tge/script/Contexts/ScriptUpdateContext.h>
#include <tge/script/BaseProperties.h>

using namespace Tga;

class FloatValueNode : public ScriptNodeBase
{
	ScriptPinId myValuePinId;

public:
	void Init(const ScriptCreationContext& context) override
	{
		ScriptPin outputPin = {};
		outputPin.type = ScriptLinkType::Property;
		outputPin.dataType = GetPropertyType<float>();
		outputPin.name = "Value"_tgaid;
		outputPin.node = context.GetNodeId();
		outputPin.role = ScriptPinRole::Output;
		context.FindOrCreatePin(outputPin);

		ScriptPin valuePin = {};
		valuePin.type = ScriptLinkType::Property;
		valuePin.dataType = GetPropertyType<float>();
		valuePin.role = ScriptPinRole::Input;
		valuePin.name = "Float"_tgaid;
		valuePin.node = context.GetNodeId();
		valuePin.defaultValue = Property::Create<float>(0.f);
		myValuePinId = context.FindOrCreatePin(valuePin);
	}

	Property ReadPin(Tga::ScriptExecutionContext& ctx, Tga::ScriptPinId) const override
	{
		return ctx.ReadInputPin(myValuePinId);
	}
};

class Float2ValueNode : public ScriptNodeBase
{
	ScriptPinId myXPinId;
	ScriptPinId myYPinId;

public:
	void Init(const ScriptCreationContext& context) override
	{
		ScriptPin outputPin = {};
		outputPin.type = ScriptLinkType::Property;
		outputPin.dataType = GetPropertyType<Vector2f>();
		outputPin.name = "Value"_tgaid;
		outputPin.node = context.GetNodeId();
		outputPin.role = ScriptPinRole::Output;
		context.FindOrCreatePin(outputPin);

		ScriptPin xPin = {};
		xPin.type = ScriptLinkType::Property;
		xPin.dataType = GetPropertyType<float>();
		xPin.role = ScriptPinRole::Input;
		xPin.name = "X"_tgaid;
		xPin.node = context.GetNodeId();
		xPin.defaultValue = Property::Create<float>(0.f);
		myXPinId = context.FindOrCreatePin(xPin);

		ScriptPin yPin = {};
		yPin.type = ScriptLinkType::Property;
		yPin.dataType = GetPropertyType<float>();
		yPin.role = ScriptPinRole::Input;
		yPin.name = "Y"_tgaid;
		yPin.node = context.GetNodeId();
		yPin.defaultValue = Property::Create<float>(0.f);
		myYPinId = context.FindOrCreatePin(yPin);
	}

	Property ReadPin(Tga::ScriptExecutionContext& ctx, Tga::ScriptPinId) const override
	{
		float x = *ctx.ReadInputPin(myXPinId).Get<float>();
		float y = *ctx.ReadInputPin(myYPinId).Get<float>();
		
		return Property::Create<Vector2f>(x, y);
	}
};

class Float3ValueNode : public ScriptNodeBase
{
	ScriptPinId myXPinId;
	ScriptPinId myYPinId;
	ScriptPinId myZPinId;

public:
	void Init(const ScriptCreationContext& context) override
	{
		ScriptPin outputPin = {};
		outputPin.type = ScriptLinkType::Property;
		outputPin.dataType = GetPropertyType<Vector3f>();
		outputPin.name = "Value"_tgaid;
		outputPin.node = context.GetNodeId();
		outputPin.role = ScriptPinRole::Output;
		context.FindOrCreatePin(outputPin);

		ScriptPin xPin = {};
		xPin.type = ScriptLinkType::Property;
		xPin.dataType = GetPropertyType<float>();
		xPin.role = ScriptPinRole::Input;
		xPin.name = "X"_tgaid;
		xPin.node = context.GetNodeId();
		xPin.defaultValue = Property::Create<float>(0.f);
		myXPinId = context.FindOrCreatePin(xPin);

		ScriptPin yPin = {};
		yPin.type = ScriptLinkType::Property;
		yPin.dataType = GetPropertyType<float>();
		yPin.role = ScriptPinRole::Input;
		yPin.name = "Y"_tgaid;
		yPin.node = context.GetNodeId();
		yPin.defaultValue = Property::Create<float>(0.f);
		myYPinId = context.FindOrCreatePin(yPin);

		ScriptPin zPin = {};
		zPin.type = ScriptLinkType::Property;
		zPin.dataType = GetPropertyType<float>();
		zPin.role = ScriptPinRole::Input;
		zPin.name = "Z"_tgaid;
		zPin.node = context.GetNodeId();
		zPin.defaultValue = Property::Create<float>(0.f);
		myZPinId = context.FindOrCreatePin(zPin);
	}

	Tga::Property ReadPin(Tga::ScriptExecutionContext& ctx, Tga::ScriptPinId) const override
	{
		float x = *ctx.ReadInputPin(myXPinId).Get<float>();
		float y = *ctx.ReadInputPin(myYPinId).Get<float>();
		float z = *ctx.ReadInputPin(myZPinId).Get<float>();

		return Property::Create<Vector3f>(x, y, z);
	}
};

class Float4ValueNode : public ScriptNodeBase
{
	ScriptPinId myValuePinId;

public:
	void Init(const ScriptCreationContext& context) override
	{
		ScriptPin outputPin = {};
		outputPin.type = ScriptLinkType::Property;
		outputPin.dataType = GetPropertyType<Vector4f>();
		outputPin.name = "Value"_tgaid;
		outputPin.node = context.GetNodeId();
		outputPin.role = ScriptPinRole::Output;
		context.FindOrCreatePin(outputPin);

		ScriptPin valuePin = {};
		valuePin.type = ScriptLinkType::Property;
		valuePin.dataType = GetPropertyType<Vector4f>();
		valuePin.role = ScriptPinRole::Input;
		valuePin.name = "Float4"_tgaid;
		valuePin.node = context.GetNodeId();
		valuePin.defaultValue = Property::Create<Vector4f>(0.f);
		myValuePinId = context.FindOrCreatePin(valuePin);
	}

	Tga::Property ReadPin(Tga::ScriptExecutionContext& ctx, Tga::ScriptPinId) const override
	{
		return ctx.ReadInputPin(myValuePinId);
	}
};

class ColorNode : public ScriptNodeBase
{
	ScriptPinId myValuePinId;

public:
	void Init(const ScriptCreationContext& context) override
	{
		ScriptPin outputPin = {};
		outputPin.type = ScriptLinkType::Property;
		outputPin.dataType = GetPropertyType<Color>();
		outputPin.name = "Value"_tgaid;
		outputPin.node = context.GetNodeId();
		outputPin.role = ScriptPinRole::Output;
		context.FindOrCreatePin(outputPin);

		ScriptPin valuePin = {};
		valuePin.type = ScriptLinkType::Property;
		valuePin.dataType = GetPropertyType<Color>();
		valuePin.role = ScriptPinRole::Input;
		valuePin.name = "Color"_tgaid;
		valuePin.node = context.GetNodeId();
		valuePin.defaultValue = Property::Create<Color>(0.f, 0.f, 0.f, 1.f);
		myValuePinId = context.FindOrCreatePin(valuePin);
	}

	Tga::Property ReadPin(Tga::ScriptExecutionContext& ctx, Tga::ScriptPinId) const override
	{
		return ctx.ReadInputPin(myValuePinId);
	}
};

void Tga::RegisterCommonMathNodes()
{
	ScriptNodeTypeRegistry::RegisterType<FloatValueNode>("Common/Math/float", "A float value");
	ScriptNodeTypeRegistry::RegisterType<Float2ValueNode>("Common/Math/float2", "A two component float");
	ScriptNodeTypeRegistry::RegisterType<Float3ValueNode>("Common/Math/float3", "A three component float");
	ScriptNodeTypeRegistry::RegisterType<Float4ValueNode>("Common/Math/float4", "A four component float");
	ScriptNodeTypeRegistry::RegisterType<ColorNode>("Common/Color", "A color node");
}
