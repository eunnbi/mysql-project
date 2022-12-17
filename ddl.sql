create table User (
    first_name varchar(50) not null,
    last_name varchar(50) not null,
    email varchar(100) primary key,
    password varchar(50) not null
);

create table Admin (
    email varchar(100) primary key,
    password varchar(50) not null
);

create table Category (
    name varchar(20) primary key
);

create table Cond (
    name varchar(30) primary key
);

create table Item (
    ID INT AUTO_INCREMENT PRIMARY KEY,
    category varchar(20) not null,
    cond varchar(30) not null,
    description varchar(50) not null,
    buy_now_price INT not null,
    ending_date DATETIME not null,
    posted_date DATETIME DEFAULT NOW(),
    seller varchar(100) not null,
    bid_end boolean DEFAULT 0,
    foreign key (seller) references User(email),
    foreign key (category) references Category(name),
    foreign key (cond) references Cond(name)
);

create table Bid (
    bidder varchar(100) not null,
    item_id INT not null,
    bid_price INT not null,
    bid_date DATETIME DEFAULT NOW(),
    primary key (bidder, item_id, bid_price),
    foreign key (bidder) references User(email),
    foreign key (item_id) references Item(id)
);

create table Keyword (
    keyword varchar(50) PRIMARY KEY,
    cnt INT not null
);

delimiter //
CREATE TRIGGER check_item
BEFORE INSERT ON Item
FOR EACH ROW
BEGIN
    if (new.ending_date <= NOW()) THEN
        set new.bid_end = 1;
    end if;
END; //

delimiter //
CREATE TRIGGER check_bid
BEFORE INSERT ON Bid
FOR EACH ROW
BEGIN
    declare price INT;
    select buy_now_price into price
    from Item
    where ID = new.item_id;

    if (new.bid_price = price) THEN
        update Item
        set bid_end = 1
        where ID = new.item_id;
    end if;
END; //

delimiter //
CREATE EVENT bid_end_event
ON SCHEDULE EVERY 1 MINUTE
STARTS TIMESTAMP(CURRENT_DATE) + INTERVAL 1 MINUTE
ON COMPLETION PRESERVE
ENABLE
DO
BEGIN
    update Item
    set bid_end = 1
    where ending_date <= NOW();
END; //


