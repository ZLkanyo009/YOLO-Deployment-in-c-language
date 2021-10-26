out = 0

f1 = open("./postprocess/txtytwth", "r")
f2 = open("/home/zldlg/yolov2-yolov3_PyTorch/log_file/postprocess/txtytwth", "r")

lines1 = f1.readlines()
lines2 = f2.readlines()
for index, line1 in enumerate(lines1):
    list1 = lines1[index]
    list2 = lines2[index]
    if "h:" in list1 and "h:" in list2 and "th:" not in list1 and "th:" not in list2:
        print(list1, end = "")
        continue
    elif list1 == "\n":
        continue
    list1 = lines1[index].strip("\n").split(":")
    list2 = lines2[index].strip("\n").split(":")

    print(list2[0],":", end = " ")
    print(float(list1[1]) - float(list2[1]))
    # if "conf_score" in list2[0]:
    #     print("")
    if "ymax_r" in list2[0]:
        print("")
    