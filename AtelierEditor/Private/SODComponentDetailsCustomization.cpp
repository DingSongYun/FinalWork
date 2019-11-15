#include "SODComponentDetailsCustomization.h"
#include "DetailCategoryBuilder.h"
#include "Materials/Material.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "PhysicsEngine/BodySetup.h"
#include "IDocumentation.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PropertyCustomizationHelpers.h"
#include "Stage/StateOfDetailComponent.h"
#include "UnrealType.h"


class FStateModelDetail
	: public IDetailCustomNodeBuilder
{
public:
	FStateModelDetail(UClass* InModelClass, UObject* InModel, TSharedPtr<IPropertyHandle> InProperty)
		: SODModel(InModel), ModelClass(InModelClass), ModelProperty(InProperty)
	{
		check(InModel)
		check(InProperty.IsValid())
		ModelProperty->MarkHiddenByCustomization();
	}

	~FStateModelDetail() {}

	virtual bool RequiresTick() const override { return false; }
	virtual void Tick( float DeltaTime ) override {}

	virtual FName GetName() const override
	{
		return ModelProperty->GetProperty()->GetFName();
	}

	virtual bool InitiallyCollapsed() const override { return true; }

	virtual void GenerateHeaderRowContent( FDetailWidgetRow& NodeRow ) override
	{
		NodeRow
			.NameContent()
			[
				ModelProperty->CreatePropertyNameWidget()
			];
	}

	void GenerateSubArrayElement(TSharedRef<IPropertyHandle> PropertyHandle, int32 ArrayIndex, IDetailChildrenBuilder& ChildrenBuilder,
		UClass* ObjClass, UObjectProperty* ObjProperty, FScriptArrayHelper ArrayHelper)
	{
		if (UObject* Obj = ObjProperty->GetObjectPropertyValue(ArrayHelper.GetRawPtr(ArrayIndex)))
		{
			TSharedRef<FStateModelDetail> Builder = MakeShareable(new FStateModelDetail(ObjClass, Obj, PropertyHandle));
			ChildrenBuilder.AddCustomBuilder(Builder);
		}
	}

	virtual void GeneratedChildStateContent(IDetailChildrenBuilder& ChildrenBuilder, TSharedPtr<IPropertyHandle> StatePropertyHandle)
	{
		UProperty* ChildProp = StatePropertyHandle->GetProperty();

		if (!StatePropertyHandle.IsValid() && ChildProp)
			return;

		// value prt of the uproperty
		const void* PropValuePtr = ChildProp->ContainerPtrToValuePtr<void>(SODModel);

		if (UArrayProperty *ArrayProperty = Cast<UArrayProperty>(ChildProp))
		{
			// array helper
			FScriptArrayHelper ScriptArrayHelper(ArrayProperty, PropValuePtr);
			if (UProperty* ArrayEleProp = ArrayProperty->Inner)
			{
				if (UObjectProperty* ObjProp = Cast<UObjectProperty>(ArrayEleProp))
				{
					UClass* ObjClass = ObjProp->PropertyClass;
					TSharedRef<FDetailArrayBuilder> StateArrayBuilder = MakeShareable(new FDetailArrayBuilder(StatePropertyHandle.ToSharedRef()));
					StateArrayBuilder->OnGenerateArrayElementWidget(FOnGenerateArrayElementWidget::CreateRaw(this, &FStateModelDetail::GenerateSubArrayElement, 
						ObjClass, ObjProp, ScriptArrayHelper));
					ChildrenBuilder.AddCustomBuilder(StateArrayBuilder);
				}
			}
		}
		else if (UObjectProperty* ObjProp = Cast<UObjectProperty>(ChildProp))
		{
			UObject* Obj = ObjProp->GetObjectPropertyValue(PropValuePtr);
			TSharedRef<FStateModelDetail> StateModelBuilder = MakeShareable(new FStateModelDetail(ObjProp->PropertyClass, Obj, StatePropertyHandle));
			ChildrenBuilder.AddCustomBuilder(StateModelBuilder);
		}
	}

	virtual void GenerateChildContent( IDetailChildrenBuilder& ChildrenBuilder ) override
	{
		check(ModelClass && SODModel)
		TArray<UObject*> ExternalObjects;
		ExternalObjects.Add(SODModel);

		// For StateModel
		for( TFieldIterator<UProperty> Prop(ModelClass); Prop; ++Prop )
		{
			if (Prop->HasAnyPropertyFlags(CPF_Edit))
			{
				IDetailPropertyRow* Row = ChildrenBuilder.AddExternalObjectProperty(ExternalObjects, Prop->GetFName());
				if (!Row) continue;
				TSharedPtr<IPropertyHandle> PropHandler = Row->GetPropertyHandle();
				if (!PropHandler.IsValid()) continue;
				PropHandler->SetOnPropertyValueChanged(OnPropertyChanged);

				if (UArrayProperty *ArrayProperty = Cast<UArrayProperty>(*Prop))
				{
					UProperty* ArrayEleProp = ArrayProperty->Inner;
					if (UObjectProperty* ObjProp = Cast<UObjectProperty>(ArrayEleProp))
					{
						UClass* ObjClass = ObjProp->PropertyClass;
						if (ObjClass->IsChildOf(USODState::StaticClass()) || ObjClass->IsChildOf(USODModel::StaticClass()))
						{
							Row->Visibility(EVisibility::Collapsed);
							GeneratedChildStateContent(ChildrenBuilder, PropHandler);
						}
					}
					else if (USoftObjectProperty* tpObjProp = Cast<USoftObjectProperty>(ArrayEleProp))
					{
						UClass* ObjClass = tpObjProp->PropertyClass;
						if (ObjClass->IsChildOf(USODState::StaticClass()) || ObjClass->IsChildOf(USODModel::StaticClass()))
						{
							Row->Visibility(EVisibility::Collapsed);
							GeneratedChildStateContent(ChildrenBuilder, PropHandler);
						}
					}
				}
				else if (UObjectProperty* ObjProp = Cast<UObjectProperty>(*Prop))
				{
					UClass* ObjClass = ObjProp->PropertyClass;
					if (ObjClass->IsChildOf(USODState::StaticClass()) || ObjClass->IsChildOf(USODModel::StaticClass()))
					{
						Row->Visibility(EVisibility::Collapsed);
						GeneratedChildStateContent(ChildrenBuilder, PropHandler);
					}
				}
			}
		}
	}

	virtual void SetOnRebuildChildren( FSimpleDelegate InOnRegenerateChildren ) { }
	
	FSimpleDelegate OnPropertyChanged;
private:
	UObject* SODModel;
	UClass* ModelClass;
	TSharedPtr<IPropertyHandle> ModelProperty;
};


/***************************************************************************/
/****************FSODComponentDetailsCustomizaiton**************************/
/***************************************************************************/
FSODComponentDetailsCustomizaiton::FSODComponentDetailsCustomizaiton()
{
}

TSharedRef<IDetailCustomization> FSODComponentDetailsCustomizaiton::MakeInstance()
{
	return MakeShareable(new FSODComponentDetailsCustomizaiton());
}

void FSODComponentDetailsCustomizaiton::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailLayout.GetObjectsBeingCustomized(ObjectsBeingCustomized);
	if (ObjectsBeingCustomized.Num() > 0)
	{
		if (UStateOfDetailComponent* Comp = Cast<UStateOfDetailComponent>(ObjectsBeingCustomized[0]))
		{
			CustomizeDetails_Internal(Comp, DetailLayout);
		}
	}
}

void FSODComponentDetailsCustomizaiton::GenerateAdditionalTextureWidget(TSharedRef<IPropertyHandle> PropertyHandle, int32 ArrayIndex, IDetailChildrenBuilder& ChildrenBuilder, class UStateOfDetailComponent* SODComponent)
{
	check(SODComponent);
	if(SODComponent->States.IsValidIndex(ArrayIndex) && SODComponent->States[ArrayIndex])
	{
		UClass* StateClass = SODComponent->StateModel->StateClass;
		TSharedRef<FStateModelDetail> StateModelBuilder = MakeShareable(new FStateModelDetail(StateClass, SODComponent->States[ArrayIndex], PropertyHandle));
		ChildrenBuilder.AddCustomBuilder(StateModelBuilder);
	}
}

void FSODComponentDetailsCustomizaiton::OnStateModelChanged(class UStateOfDetailComponent* SODComponent)
{
	check(SODComponent)
	SODComponent->RefreshStateFromModel(SODComponent->StateModel);
}

void FSODComponentDetailsCustomizaiton::CustomizeDetails_Internal(class UStateOfDetailComponent* SODComponent, IDetailLayoutBuilder& DetailLayout)
{
	if (!SODComponent) return;

	TSubclassOf<USODModel> SODModelClass = SODComponent->StateModelClass;
	if (!SODModelClass)
	{
		return ;
	}

	USODModel* SODModel = SODComponent->StateModel;

	IDetailCategoryBuilder& SODCategory = DetailLayout.EditCategory("SOD", FText::GetEmpty(), ECategoryPriority::Important);

	// UStateOfDetailComponent::StateModelClass
	SODCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UStateOfDetailComponent, StateModelClass));

	// UStateOfDetailComponent::StateModel
	TSharedPtr<IPropertyHandle> StateModelPropertyHandler = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UStateOfDetailComponent, StateModel));
	TSharedRef<FStateModelDetail> StateModelBuilder = MakeShareable(new FStateModelDetail(SODModelClass, SODModel, StateModelPropertyHandler));
	FSimpleDelegate OnPropChanged;
	OnPropChanged.BindLambda([SODComponent](){
		//UE_LOG(LogTemp, Warning, TEXT("OnStateModel Changed"));
		SODComponent->RefreshStateFromModel();
	});
	StateModelBuilder->OnPropertyChanged = OnPropChanged;
	SODCategory.AddCustomBuilder(StateModelBuilder);

	// UStateOfDetailComponent::StateNum
	SODCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UStateOfDetailComponent, StateNum));

	// UStateOfDetailComponent::States
	TSharedPtr<IPropertyHandle> StatesProperty = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UStateOfDetailComponent, States));
	TSharedRef<FDetailArrayBuilder> StateArrayBuilder = MakeShareable(new FDetailArrayBuilder(StatesProperty.ToSharedRef()));
	StateArrayBuilder->OnGenerateArrayElementWidget(FOnGenerateArrayElementWidget::CreateSP(this, &FSODComponentDetailsCustomizaiton::GenerateAdditionalTextureWidget, SODComponent));
	SODCategory.AddCustomBuilder(StateArrayBuilder);
}
