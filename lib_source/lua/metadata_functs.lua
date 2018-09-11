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


-- function boundingBoxToObjectNamesAndCounts(sim_name, timestep, ndx, ndy, ndz, ceph_obj_size, var_name, var_version, data_size, x1, y1, z1, x2, y2, z2)
function boundingBoxToObjectNamesAndCounts(get_counts, job_id, sim_name, timestep, ndx, ndy, ndz, ceph_obj_size, var_name, var_version, data_size, x1, y1, z1, x2, y2, z2, var_x1, var_x2, var_y1, var_y2, var_z1, var_z2)

    function math.round(num)
        ceil = math.ceil(num)
        floor = math.floor(num)
        if( (ceil - num) < (num - floor) ) then
            return ceil
        else
            return floor
        end
    end

    object_names = {}

    --note: could check to make sure args are safe (e.g., x1 < x2 <= nx)
    --still need to implement x<=nx, etc

    -- print(string.format("\nBounding box requested: (%d,%d,%d),(%d,%d,%d)",x1,y1,z1,x2,y2,z2))
    -- print("ndx: ", ndx, "ndy: ", ndy, "ndz: ",ndz,"data_size: ",data_size)
    -- print("var_x1: ", var_x1, "var_x2: ", var_x2, "var_y1: ",var_y1,"var_y2: ",var_y2,"var_z1: ",var_z1,"var_z2: ",var_z2)

    --this should work f
    if(x1 > x2 ) then
        x1, x2 = x2, x1
    end
    if(y1 > y2 ) then
        y1, y2 = y2, y1
    end
    if(z1 > z2 ) then
        z1, z2 = z2, z1
    end

    local chunk_size
    local y_init
    local z_init

    local x_offset = x1 - var_x1
    local y_offset = y1 - var_y1
    local z_offset = z1 - var_z1
    -- print("x_offset: ", x_offset, "y_offset: ", y_offset, "z_offset: ", z_offset)

    if(ndy == 0) then
        chunk_size = ndx * data_size
        y_init = 0
        z_init = 0
        ndy = 1
        ndz = 1
    elseif(ndz == 0) then
        chunk_size = ndx * ndy * data_size
        -- y_init = math.floor(y1 / ndy) * ndy
        y_init = math.floor(y_offset / ndy) * ndy + var_y1
        z_init = 0
        ndz = 1
    else
        chunk_size = ndx * ndy * ndz * data_size
        y_init = math.floor(y_offset / ndy) * ndy + var_y1
        -- y_init = math.floor(y1 / ndy) * ndy
        -- z_init = math.floor(z1 / ndz) * ndz
        z_init = math.floor(z_offset / ndz) * ndz + var_z1
    end

    -- print("chunk_size: ", chunk_size)
    -- print("y_init: ",y_init,"z_init: ",z_init)

    local num_objs_per_chunk = math.round(chunk_size / ceph_obj_size)
    if(num_objs_per_chunk <= 0) then
        num_objs_per_chunk = 1
    end
    local x_width = math.max( math.round(ndx / num_objs_per_chunk), 1)
    local last_x_width = ndx - x_width * (num_objs_per_chunk-1)
    if(last_x_width <= 0) then
        num_objs_per_chunk = num_objs_per_chunk + math.floor( (last_x_width-1) / x_width)
        last_x_width = ndx - x_width * (num_objs_per_chunk-1)
    end

    -- local x_chunk = math.floor(x1 / ndx) * ndx
    local x_chunk = math.floor(x_offset / ndx) * ndx + var_x1
    -- local obj_num_init = math.floor((x1 - x_chunk) / x_width)
    local obj_num_init = math.floor((x1 - x_chunk) / x_width)
    local x_init =  x_chunk + obj_num_init * x_width


    -- print("x_chunk: ",x_chunk)
    -- print("obj_num_init: ",obj_num_init)
    -- print("Decomposition information: ")
    -- print(string.format("New Bounding box requested: (%d,%d,%d),(%d,%d,%d)",x1,y1,z1,x2,y2,z2))
    -- print("chunk_size: ", chunk_size)
    -- print("num_objs_per_chunk: ",num_objs_per_chunk)
    -- print("x_width: ", x_width)
    -- print("object_size: ", x_width * ndy * ndz * data_size)
    -- if(x_width ~= last_x_width) then
    --     print("last_x_width: ",last_x_width)
    --     print("last object_size: ", last_x_width * ndy * ndz * data_size)  
    -- end
    


    local last_x 

    function x_iter ()
      local x = x_init
      local object_num = obj_num_init
      -- print("x_init: ",x_init,"object_num: ",object_num)

      return function ()
            local cur_x = x
            -- print("cur_x: ",cur_x)
            object_num = (object_num + 1) % num_objs_per_chunk
            if object_num == 0 then
                x = x + last_x_width
            else 
                x = x + x_width
            end
            if(cur_x <= x2) then
                if(get_counts) then
                    start_indx_x = math.max(x1-cur_x,0) 
                    end_indx_x = math.min(x2-cur_x,x-cur_x-1) --ndx = x-cur_x-1
                    count_x = end_indx_x-start_indx_x+1
                    -- print(string.format("obj x goes from %d to %d",cur_x,x-1))
                    -- print(string.format("want x from %d to %d",start_indx_x,end_indx_x))
                    -- print(string.format("from x %d count %d",start_indx_x,count_x))
                    table.insert(object_names, start_indx_x)
                    table.insert(object_names, count_x)
                end

                return cur_x
            else
                last_x = cur_x-1 --just for outputting decomposition info 
                return
            end
        end
    end

    for z = z_init, z2, ndz do
        for y = y_init, y2, ndy do
            for x in x_iter() do
                if(get_counts) then
                    if(y2 > 0) then
                        start_indx_y = math.max(y1-y,0) 
                        end_indx_y = math.min(y2-y,ndy-1)
                        count_y = end_indx_y-start_indx_y+1
                        -- print(y2, y, ndy)
                        -- print(string.format("obj y goes from %d to %d",y,y+ndy-1))
                        -- print(string.format("want y from %d to %d",start_indx_y,end_indx_y))
                        -- print(string.format("from y %d count %d",start_indx_y,count_y))
                        table.insert(object_names, start_indx_y)
                        table.insert(object_names, count_y)
                    end
                    if (z2 > 0) then
                        start_indx_z = math.max(z1-z,0) 
                        end_indx_z = math.min(z2-z,ndz-1)
                        count_z = end_indx_z-start_indx_z+1
                        -- print(string.format("obj z goes from %d to %d",z,z+ndz-1))
                        -- print(string.format("want z from %d to %d",start_indx_z,end_indx_z))
                        -- print(string.format("from z %d count %d",start_indx_z,count_z))
                        table.insert(object_names, start_indx_z)
                        table.insert(object_names, count_z)
                    end
                end

                name = string.format("%d/%s/%d/%s/%d/%d/%d/%d",job_id, sim_name, timestep, var_name, var_version, x, y, z)
                -- name = string.format("%d/%s/%d/%d/%d/%d",timestep, var_name, var_version, x, y, z)
                -- print("name", name)
                table.insert(object_names, name)

            end
        end
    end

    --just for outputting decomposition info
    local last_y = math.floor( (y2 - var_y1) / ndy) * ndy + ndy - 1 + var_y1
    local last_z = math.floor( (z2 - var_z1) / ndz) * ndz + ndz - 1 + var_z1

    -- print("x_init: ",x_init,"y_init: ",y_init,"z_init: ",z_init,"last_x: ",last_x,"last_y: ",last_y,"last_z: ",last_z)

    -- print(string.format("Bounding box for data retrieved: (%d,%d,%d),(%d,%d,%d)",x_init,y_init,z_init,last_x, last_y, last_z))

    return object_names
end






