function char_move(dir)
  if dir == 0 then keypress(0x26) return end --��
  if dir == 1 then keypress(0x27) return end --����
  if dir == 2 then keypress(0x28) return end --�Ʒ�
  if dir == 3 then keypress(0x25) return end --��
end

function get_bonus_exp()
	bonus_exp_1()
	sleep(1000)
	bonus_exp_2()
	sleep(1000)
	keypress(0x1b)
	sleep(1000)
end

function move_config()
  local road_config = {}
  for map_num=0, max_num do
    road_config[map_num]={}
  end

  map0 = mapname()
  if( map0 ~= map1) then road_i=0 end
  if(string.find(mapname(), "����") == nil and string.find(mapname(), "����") == nil and string.find(mapname(), "����") == nil and string.find(mapname(), "���") == nil) then--and mapname() ~= "�ο����ɼ�" and mapname() ~= "�ο���缺") then 
  	return_to_inn()
  	sleep(1000)
	return
  end

  if (string.find(mapname(), "���ƿ���") ~= nil or string.find(mapname(), "���翬��") ~= nil or string.find(mapname(), "���ɿ���") ~= nil or string.find(mapname(), "��翬��") ~= nil) then
	mapmove(0x33)
	road_i = 0
	itemOn = 0
	sleep(1000)
	return
  end

 

  if(string.find(mapname(), "�ο����Ƽ�") or string.find(mapname(), "�ο����缺") or string.find(mapname(), "�ο����ɼ�") or string.find(mapname(), "�ο���缺")) then
    road_config[0].x, road_config[0].y = 75, 148
    road_config[1].x, road_config[1].y = 75, 139
    road_config[2].x, road_config[2].y = 85, 139
    road_config[3].x, road_config[3].y = 85, 143
    road_config[4].x, road_config[4].y = 87, 143
    road_config[5].x, road_config[5].y = 87, 142
  elseif(string.find(mapname(), "��������1")) then
    road_config[0].x, road_config[0].y = 5, 15
    road_config[1].x, road_config[1].y = 5, 7
    road_config[2].x, road_config[2].y = 15, 7
    road_config[3].x, road_config[3].y = 15, 23
    road_config[4].x, road_config[4].y = 27, 23
    road_config[5].x, road_config[5].y = 27, 9
    road_config[6].x, road_config[6].y = 29, 9
  elseif(string.find(mapname(), "��������2")) then
    road_config[0].x, road_config[0].y = 2, 17
    road_config[1].x, road_config[1].y = 2, 22
    road_config[2].x, road_config[2].y = 23, 22
    road_config[3].x, road_config[3].y = 23, 16
    road_config[4].x, road_config[4].y = 24, 16
  elseif(string.find(mapname(), "��������3")) then
    road_config[0].x, road_config[0].y = 19, 15
    road_config[1].x, road_config[1].y = 19, 0
  elseif(string.find(mapname(), "��������4")) then
    road_config[0].x, road_config[0].y = 9, 6
    road_config[1].x, road_config[1].y = 22, 6
    road_config[2].x, road_config[2].y = 22, 13
    road_config[3].x, road_config[3].y = 33, 13
    road_config[4].x, road_config[4].y = 33, 0
  elseif(string.find(mapname(), "��������5")) then
    road_config[0].x, road_config[0].y = 4, 8
    road_config[1].x, road_config[1].y = 13, 8
    road_config[2].x, road_config[2].y = 13, 29
  elseif(string.find(mapname(), "��������6")) then
    road_config[0].x, road_config[0].y = 6, 5
    road_config[1].x, road_config[1].y = 19, 5
    road_config[2].x, road_config[2].y = 19, 13
    road_config[3].x, road_config[3].y = 7, 13
    road_config[4].x, road_config[4].y = 7, 23
    road_config[5].x, road_config[5].y = 24, 23
  elseif(string.find(mapname(), "��������7")) then
    road_config[0].x, road_config[0].y = 9, 22
    road_config[1].x, road_config[1].y = 9, 9
    road_config[2].x, road_config[2].y = 17, 9
  elseif(string.find(mapname(), "��������8")) then
    road_config[0].x, road_config[0].y = 12, 22
    road_config[1].x, road_config[1].y = 12, 0
  elseif(string.find(mapname(), "��������9")) then
    road_config[0].x, road_config[0].y = 20, 14
    road_config[1].x, road_config[1].y = 8, 14
    road_config[2].x, road_config[2].y = 8, 5
    road_config[3].x, road_config[3].y = 20, 5
    road_config[4].x, road_config[4].y = 33, 5
    road_config[5].x, road_config[5].y = 33, 15
  end

  if refresh_i == 10 then
    refresh()
    refresh_i = 0
  else
    refresh_i = refresh_i+1
  end
  ---------------------------------------------
  attack()
  keypress(0x30)

  if pc_hp() < 500000 then
    hp_charge()
  end

  if pc_mp() < 300000 then
    mp_charge()
  end

  if pc_exp() >= 1000000000 then
    sell_exp()
  end

  if get_item_amnt() >= 201 then
	return_to_inn()
	sleep(1000)
	say("��ȣ�� �� �Ǵ�")
	sleep(1000)
	say("ȣ�� �� �Ǵ�")
	sleep(1000)
	say("����ȣ�� �� �Ǵ�")
	sleep(1000)
	say("����ȣ�� �� �Ǵ�")
	sleep(1000)
	say("��ûȣ�� �� �Ǵ�")
	sleep(1000)
	set_item_amnt(0)
	sleep(1000)
  end
  ---------------------------------------------
  --[[
  if move_num > 20 then
	return_to_inn()
	sleep(1000)
	move_num = 0
  end
  ]]

  if itemOn == 0 then
    item_road_config = item_xy()
    for item_num=1, item_road_config[1][3]+1 do
      --debug(item_road_config[item_num][1])
      --debug(item_road_config[item_num][2])
      if item_road_config[item_num][1] ~= 0 then
        itemOn = item_num
        break
      end
    end
  end

  if pc_x() == road_config[road_i].x and pc_y() == road_config[road_i].y and road_i ~= 6 then
    -- ������ ��ǥ�� ��ȯ
    if road_config[road_i].x == 33 and road_config[road_i].y == 15 then
		return_to_inn()
		sleep(1000)
		get_bonus_exp()
		return
	end
    if(road_i >= max_num) then
      road_i = 0
    else road_i = road_i+1
    end
    map1 = mapname()


  elseif itemOn > 0 then
    if pc_x() == item_road_config[itemOn][1] and pc_y() == item_road_config[itemOn][2] then
      itemOn = 0
      eat()
      return
    end
    if math.random(0,1) == 0 then
      if pc_x() > item_road_config[itemOn][1]  then char_move(3) return end --��
      if pc_x() < item_road_config[itemOn][1]  then  char_move(1) return end --����
      if pc_y() > item_road_config[itemOn][2]  then  char_move(0) return end --��
      if pc_y() < item_road_config[itemOn][2]  then char_move(2) return end --�Ʒ�
    else

      if (pc_y() > item_road_config[itemOn][2])  then char_move(0) return end --��
      if (pc_y() < item_road_config[itemOn][2])  then char_move(2) return end --�Ʒ�
      if (pc_x() > item_road_config[itemOn][1])  then char_move(3) return end --��
      if (pc_x() < item_road_config[itemOn][1])  then char_move(1) return end --����
    end

  elseif math.random(0,1) == 0 then
    if pc_x() > road_config[road_i].x  then char_move(3) return end --��
    if pc_x() < road_config[road_i].x  then char_move(1) return end --����
    if pc_y() > road_config[road_i].y  then char_move(0) return end --��
    if pc_y() < road_config[road_i].y  then char_move(2) return end --�Ʒ�
  else
    if (pc_y() > road_config[road_i].y)  then char_move(0) return end --��
    if (pc_y() < road_config[road_i].y)  then char_move(2) return end --�Ʒ�
    if (pc_x() > road_config[road_i].x)  then char_move(3) return end --��
    if (pc_x() < road_config[road_i].x)  then char_move(1) return end --����
  end
end

function stop()
  showtext("��� ����")
  clear_interval("����")
end

function hunt_config()
  map1 = mapname()
  refresh_i = 0
  max_num = 7
  setting = 0
  road_i = 0
  itemOn = 0
  move_num = 0
  showtext("��� ����")
  --set_interval(3600, "getBonusExp")
  set_interval(0.01, "move_config")

end
add_command("����", "stop")
add_command("����", "hunt_config")