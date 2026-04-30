#pragma once

struct PlayerUpdateResult
{
	enum class Action
	{
		Shotgun,
		Revolver,
		PowerShot,
		Stunned,
		None
	};

	Action action;
	Tga::Vector2f position;
	Tga::Vector2f velocity;
};
