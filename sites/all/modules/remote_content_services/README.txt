Remote Content Services Module
------------------------------

This module proposes a solution to expose content type information via the services module. At the moment, this only has an implementation for node entities, but the api is generic enough to work with all fields and even arbitrary elements that you wish to expose.

Main features
-------------

1. Provide a UI to enable remote services for a content type. This is done by form altering the node_TYPE_form and adding a checkbox to the vertical tabs at the bottom. This can be added to other entity forms by replicating the submit handler rcs_enable_web_services().
2. Provide a default endpoint and resource to expose field data for a services-enabled content type. (http://example.com/content-types/fields.json)

To implement/override rcs classes
---------------------------------
1. Implement hook_rcs_classes(). This function reports back the names of your classes that handle an entity bundle (type) as well as classes that handle your fields. A '*' can be used as a catch all for fields.

function rcs_rcs_classes() {
  return array(
    'entities' => array(
      'node' => 'rcsEntityNode',
    ),
    'fields' => array(
      'textfield' => 'rcsFieldStandard',
      '*' => 'rcsFieldStandard',
    ),
  );
}

1a. You can also implement hook_rcs_classes_alter(&$classes) if you wish to manipulate these classes - specifically the ordering. The last class to be reported for any key will be used. 

2. To create your own entity handling, extend the abstract rcsEntity class. See rcs.class.inc for details. Primarily, you need to implement the createFields() method as well as CRUD methods. See the rcsEntityNode class for an example - this class uses createFields() to report entity fields as well as a few form elements that are coupled with nodes.

3. To create your own field types, extend the abstract rcsField class. You will need to implement createField() and autoComplete(). The createField() method exposes meta data about the field that your remote application will need to construct a form (type, label, required, cardinality, etc.). autoComplete() returns an array of autocomplete options. 

Jira Stories
------------
DG-3992, DG-3993. These two stories have to do with whitelisting fields, providing a UI and creating the content type service. This module goes quite a ways towards a solution for them.

To Do
--------
There is another story regarding auto complete fields. We still need a solution for determining whether or not a field should use autocomplete. Currently, all allowed options are included in the above service. When this grows long (e.g. Tags) we need the field to automatically report itself as an autocomplete field. We would then need a generic autocomplete endpoint/resource that can retrieve arbitrary autocomplete options. This would be easy for db fields, and a little harder for allowed values in the field definition.
