
#include "gtest/gtest.h"

#include "OPSMessage.h"
#include "XMLArchiverIn.h"

using namespace ops;

// ===============================

TEST(Test_OPSMessage, Test) {

	// Default constructed 
	OPSMessage obj1;
	EXPECT_STREQ(obj1.getTypeString().c_str(), "ops.protocol.OPSMessage ");
	EXPECT_STREQ(obj1.getKey().c_str(), "k");
	EXPECT_TRUE(obj1.isDataOwner());

	Address_T addr;
	int port;
	obj1.getSource(addr, port);
	EXPECT_EQ(port, 0);
	EXPECT_STREQ(addr.c_str(), "");

	EXPECT_EQ(obj1.getPublicationID(), 0);
	EXPECT_STREQ(obj1.getPublisherName().c_str(), "");
	EXPECT_STREQ(obj1.getTopicName().c_str(), "");
	EXPECT_EQ(obj1.getData(), nullptr);

	// Test set/get
	obj1.setPublicationID(8888);
	obj1.setPublisherName("GTest");
	obj1.setTopicName("TestTopic");
	EXPECT_EQ(obj1.getPublicationID(), 8888);
	EXPECT_STREQ(obj1.getPublisherName().c_str(), "GTest");
	EXPECT_STREQ(obj1.getTopicName().c_str(), "TestTopic");

	obj1.setSource("127.0.0.1", 8765);
	obj1.getSource(addr, port);
	EXPECT_EQ(port, 8765);
	EXPECT_STREQ(addr.c_str(), "127.0.0.1");
}

// ===============================
// Helper classes

static int OpsObject_MessageTest_Cnt = 0;

class OpsObject_MessageTest : public OPSObject
{
public:
	std::string Message;

	OpsObject_MessageTest() : OPSObject()
	{
		appendType("OpsObject_MessageTest");
		++OpsObject_MessageTest_Cnt;
	}
	~OpsObject_MessageTest()
	{
		--OpsObject_MessageTest_Cnt;
	}
	void serialize(ArchiverInOut* archive)
	{
		OPSObject::serialize(archive);
		archive->inout("Message", Message);
	}
};

class OpsObject_MessageTest_Factory : public SerializableFactory
{
	virtual Serializable* create(const TypeId_T& type)
	{
		if (type == "OpsObject_MessageTest") return new OpsObject_MessageTest();
		return nullptr;
	}
};

class RAII_MessageTest_FactoryHelper
{
public:
	OpsObject_MessageTest_Factory fact1;
	SerializableInheritingTypeFactory fact;
	RAII_MessageTest_FactoryHelper()
	{
		fact.add(&fact1);
	}
	~RAII_MessageTest_FactoryHelper()
	{
		// Note must remove fact1 since they are static above
		// otherwise fact tries to delete it at exit!!
		EXPECT_TRUE(fact.remove(&fact1));
	}
};

// ===============================
// Check dataOwner handling incl. getData/setData

TEST(Test_OPSMessage, Test_Data) {

	// Default constructed 
	OPSMessage obj1;
	EXPECT_TRUE(obj1.isDataOwner());
	EXPECT_EQ(obj1.getData(), nullptr);

	OpsObject_MessageTest* data = new OpsObject_MessageTest();
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);

	obj1.setData(data);
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);
	EXPECT_EQ(obj1.getData(), data);

	// Setting the same data again should not do anything
	obj1.setData(data);
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);
	EXPECT_EQ(obj1.getData(), data);

	// Setting another data should delete the original one
	obj1.setData(nullptr);
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 0);
	EXPECT_EQ(obj1.getData(), nullptr);

	obj1.setDataOwner(false);
	EXPECT_FALSE(obj1.isDataOwner());

	data = new OpsObject_MessageTest();
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);

	obj1.setData(data);
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);
	EXPECT_EQ(obj1.getData(), data);

	// Setting the same data again should not do anything
	obj1.setData(data);
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);
	EXPECT_EQ(obj1.getData(), data);

	// Setting another data should not delete the original since dataOwner is FALSE
	obj1.setData(nullptr);
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);
	EXPECT_EQ(obj1.getData(), nullptr);

	// Test that destructor doesn't delete object when message isn't a dataOwner
	{
		OPSMessage obj2;
		obj2.setDataOwner(false);
		obj2.setData(data);
		EXPECT_FALSE(obj2.isDataOwner());
		EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);
		EXPECT_EQ(obj2.getData(), data);
	}
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);

	// Test that destructor deletes object when message is a dataOwner
	{
		OPSMessage obj2;
		obj2.setData(data);
		EXPECT_TRUE(obj2.isDataOwner());
		EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);
		EXPECT_EQ(obj2.getData(), data);
	}
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 0);
}

TEST(Test_OPSMessage, Test_Serialize) {

	// Default constructed 
	OPSMessage obj1;
	EXPECT_TRUE(obj1.isDataOwner());
	EXPECT_EQ(obj1.getData(), nullptr);
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 0);

	std::string content(
		" <root>"
		"	<messageType>A</messageType>"
		"	<publisherPriority>B</publisherPriority>"
		"   <publicationID>123456789</publicationID>"
		"	<publisherName>GTestPub</publisherName>"
		"	<topicName>TestTopic</topicName>"
		"	<topLevelKey>Key1</topLevelKey>"
		"   <data type = \"OpsObject_MessageTest\">"
		"     <Message>Kalle</Message>"
		"   </data>"
		" </root>"
		" "
	);
	std::istringstream is(content);

	RAII_MessageTest_FactoryHelper factory;
	XMLArchiverIn arcIn(is, "root", &factory.fact);
	obj1.serialize(&arcIn);
	EXPECT_STREQ(obj1.getTypeString().c_str(), "ops.protocol.OPSMessage ");
	EXPECT_STREQ(obj1.getKey().c_str(), "k");
	EXPECT_EQ(obj1.getPublicationID(), 123456789);
	EXPECT_STREQ(obj1.getPublisherName().c_str(), "GTestPub");
	EXPECT_STREQ(obj1.getTopicName().c_str(), "TestTopic");
	ASSERT_NE(obj1.getData(), nullptr);

	OpsObject_MessageTest* data = dynamic_cast<OpsObject_MessageTest*>(obj1.getData());
	ASSERT_NE(data, nullptr);
	EXPECT_STREQ(data->Message.c_str(), "Kalle");
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);
}
