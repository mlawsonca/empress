--[[ 
 * Copyright 2018 National Technology & Engineering Solutions of
 * Sandia, LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS,
 * the U.S. Government retains certain rights in this software.
 *
 * The MIT License (MIT)
 * 
 * Copyright (c) 2018 Sandia Corporation
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *]]

function rank_to_bounding_box (npx, npy, npz, ndx, ndy, ndz, rank)
    local xpos = (rank % npx)
    local ypos = (math.floor(rank / npx) % npy)
    local zpos = (math.floor(rank / (npx * npy)) % npz)

    local xoffset = xpos * ndx
    local yoffset = ypos * ndy
    local zoffset = zpos * ndz
    -- print(rank)
    -- print(npx, npy, npz)
    -- print(ndx, ndy, ndz)
    -- print(xpos, ypos, zpos)
    -- print(xoffset, yoffset, zoffset)

    --remember: the first thing returned will be the last thing to pop off the stack
    return math.max(0,zoffset+ndz-1), zoffset, math.max(0,yoffset+ndy-1), yoffset, xoffset+ndx-1, xoffset

end