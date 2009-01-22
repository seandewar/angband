/*
 * File: identify.c
 * Purpose: Object identification and knowledge routines
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2009 Brain Bull
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#include "angband.h"


/*
 * Known is true when the "attributes" of an object are "known".
 *
 * These attributes include tohit, todam, toac, cost, and pval (charges).
 *
 * Note that "knowing" an object gives you everything that an "awareness"
 * gives you, and much more.  In fact, the player is always "aware" of any
 * item which he "knows", except items in stores.
 *
 * But having knowledge of, say, one "wand of wonder", does not, by itself,
 * give you knowledge, or even awareness, of other "wands of wonder".
 * It happens that most "identify" routines (including "buying from a shop")
 * will make the player "aware" of the object as well as "know" it.
 *
 * This routine also removes any inscriptions generated by "feelings".
 */
void object_known(object_type *o_ptr)
{
	/* Remove special inscription, if any */
	if (o_ptr->pseudo) o_ptr->pseudo = 0;

	/* The object is not "sensed" */
	o_ptr->ident &= ~(IDENT_SENSE);

	/* Clear the "Empty" info */
	o_ptr->ident &= ~(IDENT_EMPTY);

	/* Now we know about the item */
	o_ptr->ident |= (IDENT_KNOWN);
}


/*
 * The player is now aware of the effects of the given object.
 */
void object_aware(object_type *o_ptr)
{
	int i;

	/* Fully aware of the effects */
	k_info[o_ptr->k_idx].aware = TRUE;

	/* Some objects can change their "tile" when becoming aware */
	for (i = 1; i < o_max; i++)
	{
		object_type *floor_o_ptr = &o_list[i];

		/* If it's on the floor and of the right "kind" */
		if (!(floor_o_ptr->held_m_idx) && floor_o_ptr->k_idx == o_ptr->k_idx)
		{
			/* Redraw that location */
			lite_spot(floor_o_ptr->iy, floor_o_ptr->ix);
		}
	}
}



/*
 * Something has been "sampled"
 */
void object_tried(object_type *o_ptr)
{
	/* Mark it as tried (even if "aware") */
	k_info[o_ptr->k_idx].tried = TRUE;
}



/*
 * Determine whether a weapon or missile weapon is obviously {excellent} when worn.
 */
void object_id_on_wield(object_type *o_ptr)
{
	u32b f1, f2, f3;
	bool obvious = FALSE;

	/* Only deal with un-ID'd items */
	if (object_known_p(o_ptr)) return;

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	/* Find obvious things */
	if (f1 & TR1_OBVIOUS_MASK) obvious = TRUE;
	if (f3 & (TR3_LITE | TR3_TELEPATHY)) obvious = TRUE;

	if (!obvious) return;

	/* Strange messages for strange properties (this way, we don't have
	 * to give them when the item is identified).
	 *
	 * Perhaps these messages should be in a new edit file?
	 */

	if (f1 & TR1_STR)
		msg_format ("You feel strangely %s!", o_ptr->pval > 0 ? "strong" : "weak");
	if (f1 & TR1_INT)
		msg_format ("You feel strangely %s!", o_ptr->pval > 0 ? "smart" : "stupid");
	if (f1 & TR1_WIS)
		msg_format ("You feel strangely %s!", o_ptr->pval > 0 ? "wise" : "naive");
	if (f1 & TR1_DEX)
		msg_format ("You feel strangely %s!", o_ptr->pval > 0 ? "dextrous" : "clumsy");
	if (f1 & TR1_CON)
		msg_format ("You feel strangely %s!", o_ptr->pval > 0 ? "healthy" : "sickly");
	if (f1 & TR1_CHR)
		msg_format ("You feel strangely %s!", o_ptr->pval > 0 ? "cute" : "ugly");
	if (f1 & TR1_STEALTH)
		msg_format ("You feel strangely %s.", o_ptr->pval > 0 ? "stealthy" : "noisy");
	if (f1 & TR1_SPEED)
		msg_format ("You feel strangely %s.", o_ptr->pval > 0 ? "quick" : "sluggish");
	if (f1 & (TR1_BLOWS | TR1_SHOTS))
		msg_format ("Your hands strangely %s!", o_ptr->pval > 0 ? "tingle!" : "ache.");
	if (f3 & TR3_LITE)
		msg_print("It shines strangely!");
	if (f3 & TR3_TELEPATHY)
		msg_print("Your mind feels strangely sharper!");

	/* Mark the item */
	if (artifact_p(o_ptr))
	{
		if (o_ptr->pseudo != INSCRIP_TERRIBLE)
			o_ptr->pseudo = INSCRIP_SPECIAL;
	}
	else
	{
		o_ptr->pseudo = INSCRIP_EXCELLENT;
	}

	o_ptr->ident |= IDENT_SENSE;
}


/*
 * Given an object, return a short identifier which gives some idea of what
 * the item is.
 */
obj_pseudo_t object_pseudo(const object_type *o_ptr)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	if (artifact_p(o_ptr))
	{
		if (cursed_p(o_ptr))
			return INSCRIP_TERRIBLE;
		else
			return INSCRIP_SPECIAL;
	}

	if (ego_item_p(o_ptr))
	{
		if (cursed_p(o_ptr))
			return INSCRIP_WORTHLESS;
		else
			return INSCRIP_EXCELLENT;
	}

	if (cursed_p(o_ptr))
		return INSCRIP_CURSED;

	else if (o_ptr->to_a == k_ptr->to_a && o_ptr->to_h == k_ptr->to_h && o_ptr->to_d == k_ptr->to_d)
		return INSCRIP_AVERAGE;
	else if (o_ptr->to_a >= k_ptr->to_a && o_ptr->to_h >= k_ptr->to_h && o_ptr->to_d >= k_ptr->to_d)
		return INSCRIP_MAGICAL;
	else if (o_ptr->to_a <= k_ptr->to_a && o_ptr->to_h <= k_ptr->to_h && o_ptr->to_d <= k_ptr->to_d)
		return INSCRIP_MAGICAL;

	return INSCRIP_STRANGE;
}
