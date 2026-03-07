#define NOMINMAX
#include "CollisionManager.h"
#include "Function.h"
#include "Object/Boss/Boss.h"
#include "Object/Enemy/EnemyManager.h"
#include "Object/ExpCube/ExpCubeManager.h"
#include "Object/House/House.h"
#include "Object/Player/Player.h"
#include "RigidBody.h"
#include <algorithm>
namespace {
AABB MakeAabb(const Vector3& center, const Vector3& halfSize) {
	AABB aabb;
	aabb.min = {center.x - halfSize.x, center.y - halfSize.y, center.z - halfSize.z};
	aabb.max = {center.x + halfSize.x, center.y + halfSize.y, center.z + halfSize.z};
	return aabb;
}
} // namespace

void CollisionManager::HandleGameSceneCollisions(Player& player, EnemyManager& enemyManager, ExpCubeManager& expCubeManager, House& house, Boss* boss) {
	AABB playerAabb = MakeAabb(player.GetPosition(), player.GetScale());
	AABB houseAabb = MakeAabb(house.GetPosition(), house.GetScale());
	auto tryEnemyFlinch = [](Enemy* target) {
		if (!target->IsAttacking()) {
			target->Stun();
		}
	};

	for (auto& enemy : enemyManager.GetEnemies()) {
		if (!enemy->GetIsAlive()) {
			continue;
		}

		Vector3 enemyPos = enemy->GetPosition();
		AABB enemyAabb = MakeAabb(enemyPos, enemy->GetScale());
		bool hitHouseBody = RigidBody::isCollision(enemyAabb, houseAabb);
		if (hitHouseBody) {
			Vector3 toEnemy = enemyPos - house.GetPosition();
			toEnemy.y = 0.0f;
			if (LengthSquared(toEnemy) < 0.0001f) {
				toEnemy = {1.0f, 0.0f, 0.0f};
			}
			Vector3 pushDir = Function::Normalize(toEnemy);
			Vector3 houseScale = house.GetScale();
			Vector3 enemyScale = enemy->GetScale();
			float minDistance = houseScale.x + enemyScale.x;
			Vector3 correctedPos = house.GetPosition() + pushDir * minDistance;
			correctedPos.y = enemyPos.y;
			enemy->SetPosition(correctedPos);
			enemyPos = correctedPos;
			enemyAabb = MakeAabb(enemyPos, enemy->GetScale());
		}

		if (player.GetIsAlive() && player.GetSword()->IsAttacking()) {
			Vector3 swordPos = player.GetSword()->GetPosition();
			float swordHit = player.GetSword()->GetHitSize();
			AABB swordAabb = MakeAabb(swordPos, {swordHit, swordHit, swordHit});
			bool hitSword = RigidBody::isCollision(swordAabb, enemyAabb);
			if (hitSword && enemy->CanTakeDamage()) {
				enemy->SetHPSubtract(1);
				enemy->TriggerDamageInvincibility();
				enemyManager.OnEnemyDamaged(enemy.get());
				tryEnemyFlinch(enemy.get());
				if (!enemy->GetIsAlive()) {
					expCubeManager.SpawnDrops(enemy->GetPosition(), 3);
				}
			}
		}

		if (player.GetIsAlive() && player.GetSkill() && player.GetSkill()->IsDamaging()) {
			AABB skillAabb = MakeAabb(player.GetSkill()->GetDamagePosition(), player.GetSkill()->GetDamageScale());
			bool hitSkill = RigidBody::isCollision(skillAabb, enemyAabb);
			int skillDamageId = player.GetSkill()->GetSkillDamageId();
			if (hitSkill && enemy->GetLastSkillDamageId() != skillDamageId) {
				enemy->SetHPSubtract(1);
				enemy->SetLastSkillDamageId(skillDamageId);
				enemyManager.OnEnemyDamaged(enemy.get());
				tryEnemyFlinch(enemy.get());
				if (!enemy->GetIsAlive()) {
					expCubeManager.SpawnDrops(enemy->GetPosition(), 3);
				}
			}
		}

if (player.GetIsAlive() && player.GetSkill() && player.GetSkill()->IsSpecialDamaging()) {
			bool hitSpecial = false;
			for (const auto& specialTransform : player.GetSkill()->GetSpecialIceFlowerTransforms()) {
				AABB specialAabb = MakeAabb(specialTransform.translate, specialTransform.scale);
				if (RigidBody::isCollision(specialAabb, enemyAabb)) {
					hitSpecial = true;
					break;
				}
			}

			if (hitSpecial) {
				if (enemy->CanTakeDamage()) {
					enemy->SetHPSubtract(1);
					enemy->TriggerDamageInvincibility();
					enemyManager.OnEnemyDamaged(enemy.get());
					tryEnemyFlinch(enemy.get());
					if (!enemy->GetIsAlive()) {
						expCubeManager.SpawnDrops(enemy->GetPosition(), 3);
					}
				}
			}
		}

		if (enemy->IsAttackHitActive()) {
			AABB enemyAttackAabb = MakeAabb(enemy->GetAttackPosition(), {enemy->GetAttackHitSize(), enemy->GetAttackHitSize(), enemy->GetAttackHitSize()});
			const bool hitPlayer = RigidBody::isCollision(enemyAttackAabb, playerAabb);
			const bool hitHouse = RigidBody::isCollision(enemyAttackAabb, houseAabb);
			if ((hitPlayer || hitHouse) && enemy->ConsumeAttackHit()) {
				if (hitPlayer) {
					player.Damage(1);
				} else if (hitHouse) {
					house.Damage(1);
				}
			}
		}
	}
	if (boss && boss->GetIsAlive()) {
		AABB bossAabb = MakeAabb(boss->GetPosition(), boss->GetScale());

		if (player.GetIsAlive() && player.GetSword()->IsAttacking()) {
			Vector3 swordPos = player.GetSword()->GetPosition();
			float swordHit = player.GetSword()->GetHitSize();
			AABB swordAabb = MakeAabb(swordPos, {swordHit, swordHit, swordHit});
			bool hitSword = RigidBody::isCollision(swordAabb, bossAabb);
			if (hitSword && boss->CanTakeDamage()) {
				boss->SetHPSubtract(1);
				boss->TriggerDamageInvincibility();
			}
		}

		if (player.GetIsAlive() && player.GetSkill() && player.GetSkill()->IsDamaging()) {
			AABB skillAabb = MakeAabb(player.GetSkill()->GetDamagePosition(), player.GetSkill()->GetDamageScale());
			bool hitSkill = RigidBody::isCollision(skillAabb, bossAabb);
			int skillDamageId = player.GetSkill()->GetSkillDamageId();
			if (hitSkill && boss->GetLastSkillDamageId() != skillDamageId) {
				boss->SetHPSubtract(1);
				boss->SetLastSkillDamageId(skillDamageId);
			}
		}

		if (player.GetIsAlive() && player.GetSkill() && player.GetSkill()->IsSpecialDamaging()) {
			bool hitSpecial = false;
			for (const auto& specialTransform : player.GetSkill()->GetSpecialIceFlowerTransforms()) {
				AABB specialAabb = MakeAabb(specialTransform.translate, specialTransform.scale);
				if (RigidBody::isCollision(specialAabb, bossAabb)) {
					hitSpecial = true;
					break;
				}
			}

			if (hitSpecial && boss->CanTakeDamage()) {
				boss->SetHPSubtract(1);
				boss->TriggerDamageInvincibility();
			}
		}
		if (boss->IsAttackHitActive()) {
			AABB bossAttackAabb{};
			if (boss->IsChargeAttackHitActive()) {
				const Vector3 chargeHitSize = boss->GetChargeAttackHitSize();
				bossAttackAabb = MakeAabb(boss->GetPosition(), chargeHitSize);
			} else {
				const float hitSize = boss->GetAttackHitSize();
				bossAttackAabb = MakeAabb(boss->GetAttackPosition(), {hitSize, hitSize, hitSize});
			}
			const bool hitPlayer = RigidBody::isCollision(bossAttackAabb, playerAabb);
			const bool hitHouse = RigidBody::isCollision(bossAttackAabb, houseAabb);
			if ((hitPlayer || hitHouse) && boss->ConsumeAttackHit()) {
				const int damage = boss->GetAttackDamage();
				if (hitPlayer) {
					player.Damage(damage);
				} else if (hitHouse) {
					house.Damage(damage);
				}
			}
		}
	}
	if (player.GetIsAlive()) {
		for (auto& cube : expCubeManager.GetCubes()) {
			if (cube->IsCollected()) {
				continue;
			}
			const Vector3 cubePos = cube->GetPosition();
			Vector3 toCube = player.GetPosition() - cubePos;
			toCube.y = 0.0f;
			const Vector3 playerScale = player.GetScale();
			const Vector3 cubeScale = cube->GetScale();
			const float playerRadius = std::max(playerScale.x, playerScale.z);
			const float cubeRadius = std::max(cubeScale.x, cubeScale.z);
			const float pickupRadius = playerRadius + cubeRadius;
			if (LengthSquared(toCube) <= pickupRadius * pickupRadius) {
				cube->Collect();
				player.EXPMath();
			}
		}
	}
}