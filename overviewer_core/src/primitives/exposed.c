/* 
 * This file is part of the Minecraft Overviewer.
 *
 * Minecraft Overviewer is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * Minecraft Overviewer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the Overviewer.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../overviewer.h"

typedef struct {
    uint32_t mode; /* 0 = exposed only, 1 = unexposed only */
} PrimitiveExposed;

static bool
exposed_start(void* data, RenderState* state, PyObject* support) {
    PrimitiveExposed* self = (PrimitiveExposed*)data;

    if (!render_mode_parse_option(support, "mode", "I", &(self->mode)))
        return true;

    return false;
}

static bool
exposed_hidden(void* data, RenderState* state, int32_t x, int32_t y, int32_t z) {
    PrimitiveExposed* self = (PrimitiveExposed*)data;

    /* Unset these flags if seeming exposure from any of these directions would
     * be due to not having data there.
     */
    bool validMinusX = true;
    bool validPlusX = true;
    bool validMinusY = true;
    bool validPlusY = true;
    bool validMinusZ = true;
    bool validPlusZ = true;

    /* special handling for section boundaries */
    /* If the neighboring section has no block data, ignore exposure from that
     * direction 
     */
    if (x == 0 && (!(state->chunks[0][1].loaded) || state->chunks[0][1].sections[state->chunky % 16 + 1].blocks == NULL)) {
        /* No data in -x direction */
        validMinusX = false;
    }

    if (x == 15 && (!(state->chunks[2][1].loaded) || state->chunks[2][1].sections[state->chunky % 16 + 1].blocks == NULL)) {
        /* No data in +x direction */
        validPlusX = false;
    }

    if ((state->chunks[1][1].sections[state->chunky % 16].blocks == NULL)) {
        /* No data in -y direction */
        validMinusY = false;
    }

    if ((state->chunks[1][1].sections[state->chunky % 16 + 2].blocks == NULL)) {
        /* No data in +y direction */
        validPlusY = false;
    }

    if (z == 0 && (!(state->chunks[1][0].loaded) || state->chunks[1][0].sections[state->chunky % 16 + 1].blocks == NULL)) {
        /* No data in -z direction */
        validMinusZ = false;
    }

    if (z == 15 && (!(state->chunks[1][2].loaded) || state->chunks[1][2].sections[state->chunky % 16 + 1].blocks == NULL)) {
        /* No data in +z direction */
        validPlusZ = false;
    }

    /* If any of the 6 blocks adjacent to us are transparent, we're exposed */
    if ((validMinusX && is_transparent(get_data(state, BLOCKS, x - 1, y, z))) ||
        (validPlusX && is_transparent(get_data(state, BLOCKS, x + 1, y, z))) ||
        (validMinusY && is_transparent(get_data(state, BLOCKS, x, y - 1, z))) ||
        (validPlusY && is_transparent(get_data(state, BLOCKS, x, y + 1, z))) ||
        (validMinusZ && is_transparent(get_data(state, BLOCKS, x, y, z - 1))) ||
        (validPlusZ && is_transparent(get_data(state, BLOCKS, x, y, z + 1)))) {

        /* Block is exposed */
        /* Returns 1 and hides us if we're rendering unexposed blocks, 0 and 
         * shows us if we're rendering exposed blocks 
         */
        return self->mode;
    }

    /* We have no valid evidence that the block is exposed */
    return !(self->mode); /* Hide in normal mode, reveal in inverted mode */
}

RenderPrimitiveInterface primitive_exposed = {
    "exposed",
    sizeof(PrimitiveExposed),
    exposed_start,
    NULL,
    NULL,
    exposed_hidden,
    NULL,
};
