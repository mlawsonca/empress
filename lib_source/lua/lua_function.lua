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