import torch

def test_out(output, tile_row, tile_col, channel, pool, scale):
    if pool:
        tile_w = 10
        tile_h = 8
    else:
        tile_w = 20
        tile_h = 16
    
    exp_out = output
    for j_index in range(0, tile_row):
        for k_index in range(0, tile_col):
            tile_index = k_index + (j_index) * tile_col
            print("============ tile index: ", tile_index, "================")
            if j_index != (tile_row - 1):
                for i in range(0, channel):
                    for j in range(j_index * tile_h, (j_index + 1) * tile_h):
                        for k in range(k_index * tile_w, (k_index + 1) * tile_w):
                            # n c h w
                            # print("i: ", i)
                            # print("j: ", j)
                            # print("k: ", k)
                            print('{:d}\t'.format((exp_out[0][i][j][k].data * scale).int()), end = "")
                        print("")
                    print("")

            elif j_index == (tile_row - 1):
                #print("j_index == (tile_row - 1)!!!!!!")
                for i in range(0, channel):
                    for j in range(j_index * tile_h, exp_out.shape[2]):
                        for k in range(k_index * tile_w, (k_index + 1) * tile_w):
                            # n c h w
                            #print((output[0][i][j][k] * 4).int(), "   ")
                            print('{:d}\t'.format((exp_out[0][i][j][k].data * scale).int()), end = "")
                        print("")
                    print("")

f1 = open("./out/out1", "r")

out1 = torch.zeros([1, 16, 120, 160]) 
tile_row_num = 15
tile_col_num = 16

lines1 = f1.readlines()
tile_count = -1
h_count = 0
w_count = -1

w = 0
h = 0
c = -1

for index in range(0, len(lines1)):
    list1 = lines1[index].strip("\n").strip("\t").split("\t")

    for j in range(0, len(list1)):
        if len(list1) == 1:
            if list1[j] == "":
                pass
            else:
                #print(list1[j])
                if "tile" in list1[j]:
                    tile_count += 1
                    #print("tile_count: ", tile_count)
                    c = 0
                    if (tile_count) % tile_col_num == 0 and tile_count != 0:
                        h_count += 1
                        w_count = 0
                        #print("h_count: ",h_count)
                    else:
                        w_count += 1
                        #print("w_count: ",w_count)
                    h = h_count * 8
                    w = w_count * 10
        else:
            out1[0][c][h][w] = int(list1[j])
            if (h + 1 - h_count * 8) % 8 == 0 and (w + 1 - h_count * 10) % 10 == 0:
                c += 1
                h = h_count * 8
                w = w_count * 10
                
            else:
                if (w + 1 - w_count * 10) % 10 == 0:
                    h += 1
                    w = w_count * 10
                else:
                    w += 1
            #print("h: ", h, ", w: ", w, ", c: ", c)

test_out(out1, tile_row=15, tile_col=16, channel=16, pool = 1, scale=1)
