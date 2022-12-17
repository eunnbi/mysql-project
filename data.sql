delete from Admin;
delete from Bid;
delete from Item;
delete from Category;
delete from Cond;
delete from User;


insert into Admin values("1234@gmail.com", "1234"); -- admin email, password
insert into User values("John", "Smith", "john@gmail.com", "john1234");
insert into User values("Tayor", "Swift", "swift@gmail.com", "swift1234");
insert into User values("Eunbi", "Kang", "eunbi@naver.com", "eunbi1234");
insert into User values("Mina", "Lee", "mina@naver.com", "mina1234");
insert into User values("Minji", "Kang", "minji@naver.com", "minji1234");

insert into Category values("Electronics");
insert into Category values("Books");
insert into Category values("Home");
insert into Category values("Clothing");
insert into Category values("Sporting Goods");

insert into Cond values("New");
insert into Cond values("Like-New");
insert into Cond values("Used (Good)");
insert into Cond values("Used (Acceptable)");

-- bid end but no bid
insert into Item(ID, category, cond, description, buy_now_price, ending_date, posted_date, seller) values(1, "Books", "New", "Justice", 10000, "2022-11-10", "2022-10-31", "john@gmail.com");
insert into Item(ID, category, cond, description, buy_now_price, ending_date, posted_date, seller) values(2, "Clothing", "Used (Acceptable)", "Green Sweater", 9900, "2022-10-31", "2022-09-30", "mina@naver.com");

-- bid end, and sold but not buy_now_price
insert into Item(ID, category, cond, description, buy_now_price, ending_date, posted_date, seller) values(3, "Sporting Goods", "Like-New", "Golf Club", 20000, "2022-10-10", "2022-09-30", "eunbi@naver.com");
insert into Item(ID, category, cond, description, buy_now_price, ending_date, posted_date, seller) values(4, "Electronics", "New", "Mac Air M2", 100000, "2022-10-30", "2022-09-30", "eunbi@naver.com");

-- bid end, and sold at buy_now_price
insert into Item(ID, category, cond, description, buy_now_price, ending_date, posted_date, seller) values(5, "Home", "Used (Acceptable)", "Vacuum Cleaner", 120000, "2022-12-01", "2022-11-20", "swift@gmail.com"); 
insert into Item(ID, category, cond, description, buy_now_price, ending_date, posted_date, seller) values(6, "Electronics", "Like-New", "Computer Mouse", 5000, "2022-12-01", "2022-11-20", "john@gmail.com"); 

-- not end, not bid
insert into Item(ID, category, cond, description, buy_now_price, ending_date, posted_date, seller) values(7, "Home", "Used (Good)", "Couch", 20000, "2022-12-25", "2022-11-21", "mina@naver.com");

-- not end have bid
insert into Item(ID, category, cond, description, buy_now_price, ending_date, posted_date, seller) values(8, "Clothing", "Used (Good)", "Wool Coat", 10000, "2022-12-30", "2022-11-20", "swift@gmail.com");
insert into Item(ID, category, cond, description, buy_now_price, ending_date, posted_date, seller) values(9, "Sporting Goods", "Used (Acceptable)", "Badminton Stick", 10000, "2022-12-30", "2022-11-20", "eunbi@naver.com");
insert into Item(ID, category, cond, description, buy_now_price, ending_date, posted_date, seller) values(10, "Books", "Like-New", "The Hobbit", 11000, "2022-12-30", "2022-11-11", "john@gmail.com"); 

-- test event schedule
insert into Item(ID, category, cond, description, buy_now_price, ending_date, seller) values(11, "Books", "New", "The Little Prince", 10000, date_format(date_add(NOW(), INTERVAL 3 MINUTE), "%Y-%m-%d %H:%i"), "john@gmail.com");

-- bid for Item 3, 4, 5, 6, 8, 9, 10
insert into Bid values("swift@gmail.com", 3, 1000, "2022-10-01");
insert into Bid values("mina@naver.com", 3, 2000, "2022-10-06");
insert into Bid values("swift@gmail.com", 3, 5000, "2022-10-08");
insert into Bid values("mina@naver.com", 3, 6000, "2022-10-09");


insert into Bid values("john@gmail.com", 4, 10000, "2022-10-01");
insert into Bid values("swift@gmail.com", 4, 20000, "2022-10-02");
insert into Bid values("john@gmail.com", 4, 30000, "2022-10-03");
insert into Bid values("mina@naver.com", 4, 40000, "2022-10-04");
insert into Bid values("swift@gmail.com", 4, 50000, "2022-10-05");
insert into Bid values("john@gmail.com", 4, 60000, "2022-10-06");
insert into Bid values("swift@gmail.com", 4, 90000, "2022-10-07");


insert into Bid values("eunbi@naver.com", 5, 10000, "2022-11-21");
insert into Bid values("john@gmail.com", 5, 120000, "2022-11-22");

insert into Bid values("eunbi@naver.com", 6, 5000, "2022-11-21");

insert into Bid(bidder, item_id, bid_price) values("john@gmail.com", 8, 5000);
insert into Bid(bidder, item_id, bid_price) values("mina@naver.com", 8, 6000);
insert into Bid(bidder, item_id, bid_price) values("john@gmail.com", 8, 7000);
insert into Bid(bidder, item_id, bid_price) values("mina@naver.com", 8, 7500);

insert into Bid(bidder, item_id, bid_price) values("swift@gmail.com", 9, 5000);
insert into Bid(bidder, item_id, bid_price) values("mina@naver.com", 9, 6000);
insert into Bid(bidder, item_id, bid_price) values("swift@gmail.com", 9, 7000);

insert into Bid(bidder, item_id, bid_price) values("mina@naver.com", 10, 5000);
insert into Bid(bidder, item_id, bid_price) values("swift@gmail.com", 10, 6000);
insert into Bid(bidder, item_id, bid_price) values("mina@naver.com", 10, 7000);