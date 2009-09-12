/*
 * buildings.c
 * Copyright (C) Joachim de Groot 2009 <jdegroot@web.de>
 *
 * NLarn is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NLarn is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "nlarn.h"

/* stock of the dnd store */
/* TODO: make sure these items are freed on terminating the game */
static inventory *store_stock = NULL;

static const char msg_outstanding[] = "The Nlarn Revenue Service has ordered " \
                                      "us to not do business with tax evaders. " \
                                      "They have also told us that you owe back " \
                                      "taxes and, as we must comply with the " \
                                      "law, we cannot serve you at this time." \
                                      "\n\nSo Sorry.";

static int building_bank_gem_filter(item *it);
static int building_tradepost_gold_filter(item *it);

int building_bank(player *p)
{
    int turns = 2;
    char cmd;
    guint amount = 0;
    int mobuls, i;

    GPtrArray *callbacks = NULL;
    display_inv_callback *callback;

    GString *text;

    const char msg_title[] = "First National Bank of Larn";
    const char msg_greet[] = "Welcome to the First National Bank of Larn.\n\n";

    const char msg_branch[] = "Welcome to the 5th level branch office of the " \
                              "First National Bank of Larn.\n\n";

    const char msg_frozen[] = "The Larn Revenue Service has ordered that your " \
                              "account be frozen until all levied taxes have " \
                              "been paid.  They have also told us that you " \
                              "owe %d gp in taxes, and we must comply with " \
                              "them. We cannot serve you at this time.  Sorry.\n\n" \
                              "We suggest you go to the LRS office and pay your taxes.";

    assert(p != NULL);

    if (p->level->nlevel == 0)
        text = g_string_new(msg_greet);
    else
        text = g_string_new(msg_branch);

    /* leave bank when taxes are unpayed */
    if (p->outstanding_taxes)
    {
        g_string_append_printf(text,
                               msg_frozen,
                               p->outstanding_taxes);

        display_show_message((char *)msg_title, text->str);

        g_string_free(text, TRUE);
        return turns;
    }

    /* pay interest */
    mobuls = gtime2mobuls(game_turn(p->game)) - gtime2mobuls(p->interest_lasttime);

    if (p->bank_account && (mobuls > 0))
    {
        for (i = 1; i <= mobuls; i++)
            p->bank_account += p->bank_account / 250;

        p->interest_lasttime = game_turn(p->game);
    }

    g_string_append_printf(text,
                           "You have %d gold pieces in the bank.\n" \
                           "You have %d gold pieces.\n\n",
                           p->bank_account,
                           player_get_gold(p));

    g_string_append(text, "Your wish? ");

    if (player_get_gold(p) > 0)
        g_string_append(text, "d)eposit ");

    if (p->bank_account > 0)
        g_string_append(text, "w)ithdraw ");

    /* if player has gems, enable selling them */
    if (inv_item_count(p->inventory, IT_GEM, GT_NONE))
    {
        g_string_append(text, "s)ell a gem");

        /* define callback functions */
        callbacks = g_ptr_array_new();

        callback = g_malloc(sizeof(display_inv_callback));
        callback->description = "(s)ell";
        callback->key = 's';
        callback->function = &player_item_sell;
        callback->checkfun = &player_item_is_sellable;
        callback->active = FALSE;
        g_ptr_array_add(callbacks, callback);
    }

    cmd = display_show_message((char *)msg_title, text->str);
    g_string_free(text, TRUE);

    /* repaint screen (otherwise background would be black) */
    display_paint_screen(p);

    switch (cmd)
    {
    case 'd': /* deposit */
        amount = display_get_count("How many gold pieces do you wish to deposit?",
                                   player_get_gold(p));

        if (amount && (amount <= player_get_gold(p)))
        {
            p->bank_account += amount;
            player_set_gold(p, player_get_gold(p) - amount);
            log_add_entry(p->log, "You deposited %d gp.", amount);
        }
        else if (amount)
        {
            log_add_entry(p->log, "You don't have that much.");
        }

        turns += 2;

        break;

    case 'w': /* withdraw */
        amount = display_get_count("How many gold pieces do you wish to withdraw?",
                                   p->bank_account);

        if (amount && (amount <= p->bank_account))
        {
            player_set_gold(p, player_get_gold(p) + amount);
            p->bank_account -= amount;
            log_add_entry(p->log, "You withdraw %d gp.", amount);
        }
        else if (amount)
        {
            log_add_entry(p->log, "You don't have that much in the bank!");
        }

        turns += 2;

        break;

    case 's': /* sell gem */
        if (!inv_item_count(p->inventory, IT_GEM, GT_NONE))
            break;

        display_inventory("Sell gems", p, p->inventory, callbacks, TRUE,
                          &building_bank_gem_filter);

        break;

    default:
        /* do nothing */
        break;
    }

    if (callbacks)
    {
        /* clean up */
        display_inv_callbacks_clean(callbacks);
    }

    return turns;
}

int building_dndstore(player *p)
{
    int turns = 2;
    GPtrArray *callbacks;
    display_inv_callback *callback;

    const char title[] = "DND store";

    const char msg_welcome[] = "Welcome to the Nlarn Thrift Shoppe.\n" \
                               "We stock many items explorers find useful in " \
                               "their adventures. Feel free to browse to your " \
                               "heart's content. Also be advised that if you " \
                               "break 'em, you pay for 'em.";

    assert(p != NULL);

    /* no business if player has outstanding taxes */
    if (p->outstanding_taxes)
    {
        display_show_message((char *)title, (char *)msg_outstanding);
        return turns;
    }

    /* define callback functions */
    callbacks = g_ptr_array_new();

    callback = g_malloc(sizeof(display_inv_callback));
    callback->description = "(b)uy";
    callback->key = 'b';
    callback->function = &player_item_buy;
    callback->checkfun = &player_item_is_affordable;
    callback->active = FALSE;
    g_ptr_array_add(callbacks, callback);

    display_show_message((char *)title, (char *)msg_welcome);
    display_paint_screen(p);

    display_inventory((char *)title, p, store_stock, callbacks, TRUE, NULL);

    /* clean up */
    display_inv_callbacks_clean(callbacks);

    return turns;
}

void building_dndstore_init()
{
    int loop, count;
    item_t type = IT_NONE;

    /* do nothing if the store has already been initialized */
    if (store_stock)
        return;

    for (type = IT_ARMOUR; type < IT_MAX; type++)
    {
        if (type == IT_GOLD || type == IT_GEM || type == IT_CONTAINER)
        {
            continue;
        }

        if (item_is_stackable(type) && (type != IT_BOOK))
        {
            count = 3;
        }
        else
        {
            count = 1;
        }

        for (loop = 0; loop < count; loop++)
        {
            int id;

            for (id = 1; id < item_max_id(type); id++)
            {
                item *it = item_new(type, id, 0);

                /* do not generate unobtainable weapons */
                if ((type == IT_WEAPON) && !weapon_is_obtainable(it))
                {
                    item_destroy(it);
                }
                else
                {
                    inv_add(&store_stock, it);
                }
            }
        }
    }

}

void building_dndstore_item_add(item *i)
{
    assert (i != NULL);
    inv_add(&store_stock, i);
}

void building_dndstore_item_del(item *i)
{
    assert (i != NULL);
    inv_del_element(&store_stock, i);
}

int building_home(player *p)
{
    int turns = 2;
    GString *text;

    const char msg_home[] = "Welcome home, %s.\n\nLatest word from the doctor " \
                            "is not good. The diagnosis is confirmed as " \
                            "dianthroritis. He guesses that your daughter " \
                            "has only %d mobuls left in this world.  It's " \
                            "up to you, %s, to find the only hope for your " \
                            "daughter, the very rare potion of cure " \
                            "dianthroritis.  It is rumored that only deep in " \
                            "the depths of the caves can this potion be found.\n";

    const char msg_found[] = "Congratulations. You found the potion of cure " \
                             "dianthroritis! Frankly, No one thought you " \
                             "could do it. Boy! Did you surprise them!\n\n";

    const char msg_died[] = "The doctor has the sad duty to inform you that " \
                            "your daughter died before your return. There was " \
                            "nothing he could do without the potion.\n\n" \
                            "You didn't make it in time. Too bad...";

    const char msg_won[] = "The doctor is now administering the potion and, in " \
                           "a few moments, your daughter should be well on " \
                           "her way to recovery.\n\n The potion is working! " \
                           "The doctor thinks that your daughter will recover " \
                           "in a few days.\n\nCongratulations!";

    assert(p != NULL);

    /* look for potion in player's inventory */

    if (inv_item_count(p->inventory, IT_POTION, PO_CURE_DIANTHR))
    {
        /* carrying the potion */
        text = g_string_new(msg_found);

        if (game_turn(p->game) < TIMELIMIT)
        {
            /* won the game */
            g_string_append(text, msg_won);
            display_show_message("You saved your daughter!", text->str);
            g_string_free(text, TRUE);

            display_paint_screen(p);
            player_die(p, PD_WON, 0);
        }
        else
        {
            /* lost the game */
            g_string_append(text, msg_died);
            display_show_message("You were too late!", text->str);
            g_string_free(text, TRUE);

            display_paint_screen(p);
            player_die(p, PD_TOO_LATE, 0);
        }
    }
    else if (game_turn(p->game) > TIMELIMIT)
    {
        /* too late, no potion */
        text = g_string_new(msg_died);
        display_show_message("You were too late!", text->str);
        g_string_free(text, TRUE);

        display_paint_screen(p);
        player_die(p, PD_LOST, 0);
    }
    else
    {
        /* casual visit, report remaining time */
        text = g_string_new(NULL);
        g_string_printf(text,
                        msg_home,
                        p->name,
                        gtime2mobuls(game_remaining_turns(p->game)),
                        p->name);

        display_show_message("Your home", text->str);
        g_string_free(text, TRUE);
    }

    return turns;
}

int building_lrs(player *p)
{
    int turns = 2;
    GString *text;

    const char msg_greet[] = "Welcome to the Larn Revenue Service district office.\n\n";
    const char msg_taxes[] = "You presently owe %d gp in taxes.";
    const char msg_notax[] = "You do not owe us any taxes.";

    assert(p != NULL);

    text = g_string_new(msg_greet);

    if (p->outstanding_taxes)
        g_string_append_printf(text,
                               msg_taxes,
                               p->outstanding_taxes);
    else
        g_string_append(text, msg_notax);

    if (p->outstanding_taxes > player_get_gold(p))
        g_string_append(text, " You cannot afford to pay your taxes this time.");

    display_show_message("Larn Revenue Service", text->str);

    g_string_free(text, TRUE);

    /* offer to pay taxes if player can afford to */
    if (p->outstanding_taxes && (p->outstanding_taxes <= player_get_gold(p)))
    {
        /* need to redraw screen first */
        display_paint_screen(p);

        if (display_get_yesno("Do you want to pay your taxes?", NULL, NULL))
        {
            player_set_gold(p, player_get_gold(p) - p->outstanding_taxes);
            p->outstanding_taxes = 0;
            log_add_entry(p->log, "You have payed your taxes.");
        }
        else
        {
            log_add_entry(p->log, "You chose not to pay your taxes.");
        }
    }

    return turns;
}

int building_school(player *p)
{
    int turns = 2;
    guint price;

    GString *text;
    int i;
    char selection;

    const char msg_greet[] = "The College of Larn offers the exciting " \
                             "opportunity of higher education to all " \
                             "inhabitants of the caves.\n\n" \
                             "Here is a list of the class schedule:\n\n";

    const char msg_price[] = "\nAll courses cost %d gold pieces.";

    const char msg_prerequisite[] = "Sorry, but this class has a prerequisite of %s.";

    /* school courses */
    const school_course school_courses[SCHOOL_COURSE_COUNT] =
    {
        { 10, "Fighters Training I", " You feel stronger!" },
        { 15, "Fighters Training II", "You feel much stronger!" },
        { 10, "Introduction to Wizardry", "The task before you now seems more attainable!" },
        { 20, "Applied Wizardry", "The task before you now seems very attainable!" },
        { 10, "Behavioral Psychology", "You now feel like a born leader!" },
        { 10, "Faith for Today", "You now feel more confident that you can find the potion in time!" },
        { 10, "Contemporary Dance", "You feel like dancing!" },
        {  5, "History of Larn", "Your instructor told you that the Eye of Larn is rumored to be guarded by a platinum dragon who possesses psionic abilities." },
    };

    assert(p != NULL);

    /* courses become more expensive with rising difficulty */
    price = 250 + (game_difficulty(p->game) * 50);

    text = g_string_new(msg_greet);

    for (i = 0; i < SCHOOL_COURSE_COUNT; i++)
    {
        if (!p->school_courses_taken[i])
        {
            g_string_append_printf(text,
                                   "  %c) %-30s (%2d mobuls)\n",
                                   i + 'a',
                                   school_courses[i].description,
                                   school_courses[i].course_time);
        }
        else
        {
            g_string_append(text, "\n");
        }
    }

    g_string_append_printf(text, msg_price, price);

    selection = display_show_message("School", text->str);
    g_string_free(text, TRUE);

    selection -= 'a';

    if ((selection >= 0)
            && ((selection) < SCHOOL_COURSE_COUNT)
            && !p->school_courses_taken[(int)selection])
    {
        if (player_get_gold(p) < price)
        {
            log_add_entry(p->log,
                          "You cannot afford the %d for the course.",
                          price);

            return turns;

        }

        switch (selection)
        {
        case 0:
            p->strength += 2;
            p->constitution++;
            break;

        case 1:
            if (!p->school_courses_taken[0])
            {
                log_add_entry(p->log,
                              (char *)msg_prerequisite,
                              school_courses[0].description);

                return turns;
            }

            p->strength += 2;
            p->constitution += 2;
            break;

        case 2:
            p->intelligence += 2;
            break;

        case 3:
            if (!p->school_courses_taken[2])
            {
                log_add_entry(p->log,
                              (char *)msg_prerequisite,
                              school_courses[2].description);

                return turns;
            }
            p->intelligence += 2;
            break;

        case 4:
            p->charisma += 3;
            break;

        case 5:
            p->wisdom += 2;
            break;

        case 6:
            p->dexterity += 3;
            break;

        case 7:
            p->intelligence++;
            break;

        }

        /* mark course as taken */
        p->school_courses_taken[(int)selection] = 1;

        /* charge */
        player_set_gold(p, player_get_gold(p) - price);

        /* time usage */
        turns += mobuls2gtime(school_courses[(int)selection].course_time);

        log_add_entry(p->log,
                      "You take the course %s.",
                      school_courses[(int)selection].description);

        log_add_entry(p->log, school_courses[(int)selection].message);
    }

    return turns;
}

int building_tradepost(player *p)
{
    int turns = 2;
    GPtrArray *callbacks;
    display_inv_callback *callback;

    const char title[] = "Trade Post";

    const char msg_welcome[] = "Welcome to the Nlarn Trading Post.\n\nWe buy " \
                               "items that explorers no longer find useful.\n" \
                               "Since the condition of the items you bring in " \
                               "is not certain, and we incur great expense in " \
                               "reconditioning the items, we usually pay only " \
                               "20% of their value were they to be new.\n" \
                               "If the items are badly damaged, we will pay " \
                               "only 10% of their new value.";

    assert(p != NULL);

    /* no business if player has outstanding taxes */
    if (p->outstanding_taxes)
    {
        display_show_message((char *)title, (char *)msg_outstanding);
        return turns;
    }

    /* define callback functions */
    callbacks = g_ptr_array_new();

    callback = g_malloc(sizeof(display_inv_callback));
    callback->description = "(s)ell";
    callback->key = 's';
    callback->function = &player_item_sell;
    callback->checkfun = &player_item_is_sellable;
    callback->active = FALSE;
    g_ptr_array_add(callbacks, callback);

    callback = g_malloc(sizeof(display_inv_callback));
    callback->description = "(i)dentify";
    callback->key = 'i';
    callback->function = &player_item_shop_identify;
    callback->checkfun = &player_item_is_identifiable;
    callback->active = FALSE;
    g_ptr_array_add(callbacks, callback);

    callback = g_malloc(sizeof(display_inv_callback));
    callback->description = "(r)epair";
    callback->key = 'r';
    callback->function = &player_item_shop_repair;
    callback->checkfun = &player_item_is_damaged;
    callback->active = FALSE;
    g_ptr_array_add(callbacks, callback);

    callback = g_malloc(sizeof(display_inv_callback));
    callback->description = "(u)nequip";
    callback->key = 'u';
    callback->function = &player_item_unequip;
    callback->checkfun = &player_item_is_equipped;
    callback->active = FALSE;
    g_ptr_array_add(callbacks, callback);

    display_show_message((char *)title, (char *)msg_welcome);
    display_paint_screen(p);

    display_inventory((char *)title, p, p->inventory, callbacks, TRUE, &building_tradepost_gold_filter);

    /* clean up */
    display_inv_callbacks_clean(callbacks);

    return turns;
}

static int building_bank_gem_filter(item *it)
{
    assert (it != NULL);

    return (it->type == IT_GEM);
}

static int building_tradepost_gold_filter(item *it)
{
    assert (it != NULL);

    return (it->type != IT_GOLD);
}
