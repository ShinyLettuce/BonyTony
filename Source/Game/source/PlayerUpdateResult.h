#pragma once

struct PlayerUpdateResult
{
	enum class Action
	{
		Shotgun,
		Revolver,
		PowerShot,
		Stunned,
		WaddleLeft,
		WaddleRight,
		None
	};

	Action action;
	Tga::Vector2f position;
	Tga::Vector2f velocity;
};
