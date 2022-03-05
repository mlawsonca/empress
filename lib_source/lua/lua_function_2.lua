function rank_to_bounding_box (npx, npy, npz, ndx, ndy, ndz, rank)
    local xpos = rank % npx
    local ypos = math.floor(rank / npx) % npy
    local zpos = (math.floor(rank / (npx * npy)) % npz)

    local xoffset = xpos * ndx + 2
    local yoffset = ypos * ndy + 2
    local zoffset = zpos * ndz + 2
    -- print(ndx, ndy, ndz)
    -- print(xpos, ypos, zpos)
    -- print(xoffset, yoffset, zoffset)

    --remember: the first thing returned will be the last thing to pop off the stack
    return zoffset+ndz-1, zoffset, yoffset+ndy-1, yoffset, xoffset+ndx-1, xoffset

end